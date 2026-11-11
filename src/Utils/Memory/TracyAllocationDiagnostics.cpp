#include <Utils/Memory/TracyAllocationDiagnostics.h>

#ifdef SR_TRACY_ALLOCATION_DIAGNOSTICS
    #include <Utils/Common/Breakpoint.h>
#endif

#if defined(SR_TRACY_ALLOCATION_DIAGNOSTICS) && defined(SR_WIN32)
    #include <Windows.h>
    #ifdef SR_COMMON_USE_MINHOOK
        #include <MinHook.h>
    #endif
#endif

#ifdef SR_TRACY_ALLOCATION_DIAGNOSTICS
namespace SR_UTILS_NS::TracyAllocationDiagnostics {
    constexpr uint32_t BucketCount = 65536;
    constexpr uint32_t AllocationCapacity = 131072;
    constexpr uint32_t StackCapacity = 16;

    struct Allocation {
        void* address = nullptr;
        SR_UTILS_NS::SizeType size = 0;
        uint64_t sequence = 0;
        uint32_t threadId = 0;
        uint32_t stackDepth = 0;
        void* stack[StackCapacity] = {};
    };

    struct Entry {
        Allocation allocation;
        uint32_t next = 0;
    };

    enum class FailureReason : uint8_t {
        None,
        DuplicateAllocation,
        FreeOutsideSRFree,
        TableFull
    };

    // Inspect this snapshot when the debugger stops. Raw stack addresses require no symbol resolution here.
    struct Failure {
        FailureReason reason = FailureReason::None;
        Allocation first;
        Allocation repeated;
    };

    static Failure g_failure;
    static Entry g_entries[AllocationCapacity];
    static uint32_t g_buckets[BucketCount] = {};
    static uint32_t g_unusedEntry = 0;
    static uint32_t g_freeEntry = 0;
    static uint64_t g_sequence = 0;
    static std::atomic_flag g_lock = ATOMIC_FLAG_INIT;

    static void Lock() {
        while (g_lock.test_and_set(std::memory_order_acquire)) { }
    }

    static void Unlock() {
        g_lock.clear(std::memory_order_release);
    }

    static uint32_t Bucket(void* address) {
        uintptr_t hash = reinterpret_cast<uintptr_t>(address);
        hash ^= hash >> 16;
        hash *= static_cast<uintptr_t>(0x9e3779b1u);
        hash ^= hash >> 16;
        return static_cast<uint32_t>(hash) & (BucketCount - 1);
    }

    static Allocation Capture(void* address, SR_UTILS_NS::SizeType size) {
        Allocation allocation;
        allocation.address = address;
        allocation.size = size;
        allocation.sequence = ++g_sequence;
    #ifdef SR_WIN32
        allocation.threadId = GetCurrentThreadId();
        allocation.stackDepth = CaptureStackBackTrace(0, StackCapacity, allocation.stack, nullptr);
    #endif
        return allocation;
    }

#ifdef SR_WIN32
    static void WriteDiagnostic(const char* buffer, uint32_t length) {
        OutputDebugStringA(buffer);
        DWORD written = 0;
        HANDLE errorOutput = GetStdHandle(STD_ERROR_HANDLE);
        if (errorOutput && errorOutput != INVALID_HANDLE_VALUE) {
            WriteFile(errorOutput, buffer, length, &written, nullptr);
        }
        HANDLE file = CreateFileW(L"tracy-allocation-failure.log", FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            WriteFile(file, buffer, length, &written, nullptr);
            CloseHandle(file);
        }
    }
#endif

    static void ReportFailure(const Failure& failure) {
    #ifdef SR_WIN32
        char buffer[8192] = {};
        uint32_t length = 0;
        const auto append = [&](const char* text) {
            while (*text && length + 1 < sizeof(buffer)) {
                buffer[length++] = *text++;
            }
        };
        const auto number = [&](uint64_t value, uint32_t base) {
            char digits[32];
            uint32_t count = 0;
            do {
                digits[count++] = "0123456789abcdef"[value % base];
                value /= base;
            } while (value);
            while (count && length + 1 < sizeof(buffer)) {
                buffer[length++] = digits[--count];
            }
        };
        const auto allocation = [&](const char* label, const Allocation& value) {
            append(label);
            append(": address=0x");
            number(reinterpret_cast<uintptr_t>(value.address), 16);
            append(" size=");
            number(value.size, 10);
            append(" sequence=");
            number(value.sequence, 10);
            append(" threadId=");
            number(value.threadId, 10);
            append(" stackDepth=");
            number(value.stackDepth, 10);
            append("\r\n");
            for (uint32_t frame = 0; frame < value.stackDepth && frame < StackCapacity; ++frame) {
                append("  #");
                number(frame, 10);
                append(" 0x");
                number(reinterpret_cast<uintptr_t>(value.stack[frame]), 16);
                append("\r\n");
            }
        };

        append("\r\n[Tracy allocation diagnostics] ");
        switch (failure.reason) {
            case FailureReason::DuplicateAllocation: append("DuplicateAllocation"); break;
            case FailureReason::FreeOutsideSRFree: append("FreeOutsideSRFree"); break;
            case FailureReason::TableFull: append("TableFull"); break;
            default: append("Unknown"); break;
        }
        append("\r\nprocessId=");
        number(GetCurrentProcessId(), 10);
        append(" executableBase=0x");
        number(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)), 16);
        append("\r\n");
        allocation("first", failure.first);
        allocation("repeated", failure.repeated);
        buffer[length] = '\0';

        // Avoid CRT formatting, streams, symbol resolution, and the engine logger in this path.
        WriteDiagnostic(buffer, length);
    #else
        (void)failure;
    #endif
    }

    void OnAllocation(void* address, SR_UTILS_NS::SizeType size) {
        if (!address) {
            return;
        }

        Lock();
        const uint32_t bucket = Bucket(address);
        for (uint32_t index = g_buckets[bucket]; index; index = g_entries[index - 1].next) {
            if (g_entries[index - 1].allocation.address == address) {
                g_failure.reason = FailureReason::DuplicateAllocation;
                g_failure.first = g_entries[index - 1].allocation;
                g_failure.repeated = Capture(address, size);
                const Failure failure = g_failure;
                Unlock();
                ReportFailure(failure);
                SR_MAKE_BREAKPOINT;
                return;
            }
        }

        uint32_t index = g_freeEntry;
        if (index) {
            g_freeEntry = g_entries[index - 1].next;
        }
        else if (g_unusedEntry < AllocationCapacity) {
            index = ++g_unusedEntry;
        }
        else {
            g_failure.reason = FailureReason::TableFull;
            g_failure.first = Allocation{};
            g_failure.repeated = Capture(address, size);
            const Failure failure = g_failure;
            Unlock();
            ReportFailure(failure);
            SR_MAKE_BREAKPOINT;
            return;
        }

        Entry& entry = g_entries[index - 1];
        entry.allocation = Capture(address, size);
        entry.next = g_buckets[bucket];
        g_buckets[bucket] = index;
        Unlock();
    }

    void OnFree(void* address) {
        if (!address) {
            return;
        }

        Lock();
        uint32_t* link = &g_buckets[Bucket(address)];
        while (*link) {
            const uint32_t index = *link;
            Entry& entry = g_entries[index - 1];
            if (entry.allocation.address == address) {
                *link = entry.next;
                entry.next = g_freeEntry;
                g_freeEntry = index;
                break;
            }
            link = &entry.next;
        }
        // Allocations made before profiler startup are intentionally absent.
        Unlock();
    }

    void OnCrtFree(void* address) {
        static SR_THREAD_LOCAL bool checking = false;
        if (!address || checking) {
            return;
        }
        checking = true;
        Lock();
        for (uint32_t index = g_buckets[Bucket(address)]; index; index = g_entries[index - 1].next) {
            if (g_entries[index - 1].allocation.address == address) {
                g_failure.reason = FailureReason::FreeOutsideSRFree;
                g_failure.first = g_entries[index - 1].allocation;
                g_failure.repeated = Capture(address, g_failure.first.size);
                const Failure failure = g_failure;
                Unlock();
                ReportFailure(failure);
                SR_MAKE_BREAKPOINT;
                checking = false;
                return;
            }
        }
        Unlock();
        checking = false;
    }

#if defined(SR_WIN32) && defined(SR_COMMON_USE_MINHOOK)
    using FreeDbgFunction = void (__cdecl*)(void*, int32_t);
    static FreeDbgFunction g_realFreeDbg = nullptr;

    static void __cdecl HookFreeDbg(void* address, int32_t blockType) {
        if (g_TracyAllocatorInitialized) {
            OnCrtFree(address);
        }
        g_realFreeDbg(address, blockType);
    }

    void InitializeCrtHooks(void* crt) {
        const char* message = nullptr;
        void* address = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(crt), "_free_dbg"));
        if (!address) {
            message = "[Tracy allocation diagnostics] CRT has no _free_dbg export.\r\n";
        }
        else {
            const MH_STATUS status = MH_CreateHook(address, &HookFreeDbg, reinterpret_cast<void**>(&g_realFreeDbg));
            message = status == MH_OK && MH_EnableHook(address) == MH_OK
                ? "[Tracy allocation diagnostics] CRT _free_dbg hook enabled.\r\n"
                : "[Tracy allocation diagnostics] CRT _free_dbg hook installation failed.\r\n";
        }
        uint32_t length = 0;
        while (message[length]) {
            ++length;
        }
        WriteDiagnostic(message, length);
    }
#endif
}
#endif
