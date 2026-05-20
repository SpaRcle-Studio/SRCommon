//
// Created by Monika on 16.05.2026.
//

#include <Utils/Platform/WindowsMemoryAllocationHooks.h>
#include <Utils/Profile/TracyContext.h>

#ifdef SR_COMMON_USE_MINHOOK

#include <MinHook.h>

namespace SR_UTILS_NS {
    namespace MemoryHookDetails {
        using malloc_t = void* (__cdecl*)(size_t);
        using free_t   = void  (__cdecl*)(void*);

        static malloc_t real_malloc = nullptr;
        static free_t real_free = nullptr;

        using VirtualAlloc_t = LPVOID(WINAPI*)(LPVOID, SIZE_T, DWORD, DWORD);
        using VirtualFree_t = BOOL(WINAPI*)(LPVOID, SIZE_T, DWORD);

        static VirtualAlloc_t real_VirtualAlloc = nullptr;
        static VirtualFree_t real_VirtualFree = nullptr;

        using HeapAlloc_t = LPVOID(WINAPI*)(HANDLE, DWORD, SIZE_T);
        using HeapFree_t = BOOL(WINAPI*)(HANDLE, DWORD, LPVOID);

        static HeapAlloc_t real_HeapAlloc = nullptr;
        static HeapFree_t real_HeapFree = nullptr;

        static const wchar_t* g_allowedThreads[] = {
            L"Main thread",
            L"Engine",
            L"Script system",
            L"Resources manager"
        };

        extern "C" typedef HRESULT (WINAPI *t_SetThreadDescription)( HANDLE, PCWSTR );
        extern "C" typedef HRESULT (WINAPI *t_GetThreadDescription)( HANDLE, PWSTR* );
        static auto _SetThreadDescription = (t_SetThreadDescription)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "SetThreadDescription" );
        static auto _GetThreadDescription = (t_GetThreadDescription)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetThreadDescription" );

        std::atomic<bool> g_recursionGuard = false;

        bool IgnoreTrace() {
            bool expected = false;
            if (!g_recursionGuard.compare_exchange_strong(expected, true)) {
                return true;
            }

            bool allowed = false;
            PWSTR desc = nullptr;
            if (SUCCEEDED(_GetThreadDescription(GetCurrentThread(), &desc))) {
                for (const wchar_t* allowedThread : g_allowedThreads) {
                    if (wcscmp(desc, allowedThread) == 0) {
                        allowed = true;
                        break;
                    }
                }
                LocalFree(desc);
            }
            g_recursionGuard.store(false, std::memory_order_release);
            return !allowed;
        }

        void* __cdecl hook_malloc(size_t size) {
            void* p = real_malloc(size);
            SR_UTILS_NS::OnMemoryAllocated(size);
            if (g_TracyAllocatorInitialized && !IgnoreTrace()) {
                SR_TRACY_ZONE;
                SR_TRACY_PLOT("Heap", static_cast<int64_t>(SR_UTILS_NS::GetApplicationHeapSize()));
            }
            return p;
        }

        void __cdecl hook_free(void* p) {
            SR_UTILS_NS::OnMemoryFreed(p ? _msize(p) : 0);
            if (g_TracyAllocatorInitialized && !IgnoreTrace()) {
                SR_TRACY_ZONE;
                SR_TRACY_PLOT("Heap", static_cast<int64_t>(SR_UTILS_NS::GetApplicationHeapSize()));
            }
            real_free(p);
        }

        LPVOID WINAPI hook_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
            //SR_UTILS_NS::OnMemoryAllocated(static_cast<int64_t>(dwBytes));
            auto p = real_HeapAlloc(hHeap, dwFlags, dwBytes);
            if (g_TracyAllocatorInitialized && !IgnoreTrace()) {
                SR_TRACY_ZONE;
                SR_TRACY_PLOT("Heap", static_cast<int64_t>(SR_UTILS_NS::GetApplicationHeapSize()));
            }
            return p;
        }

        BOOL WINAPI hook_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
            //SR_UTILS_NS::OnMemoryFreed(static_cast<int64_t>(HeapSize(hHeap, 0, lpMem)));
            if (g_TracyAllocatorInitialized && !IgnoreTrace()) {
                SR_TRACY_ZONE;
                SR_TRACY_PLOT("Heap", static_cast<int64_t>(SR_UTILS_NS::GetApplicationHeapSize()));
            }
            return real_HeapFree(hHeap, dwFlags, lpMem);
        }

        LPVOID WINAPI hook_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
            auto p = real_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
            //SR_UTILS_NS::OnMemoryAllocated(static_cast<int64_t>(dwSize));
            return p;
        }

        BOOL WINAPI hook_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
            //SR_UTILS_NS::OnMemoryFreed(static_cast<int64_t>(dwSize));
            return real_VirtualFree(lpAddress, dwSize, dwFreeType);
        }
    }

    void InitMemoryHooks() {
        SR_TRACY_PLOT_CONFIG_MEMORY("Heap");

        if (MH_Initialize() != MH_OK) {
            SR_PLATFORM_NS::WriteConsoleError("InitMemoryHooks() : failed to initialize MinHook!\n");
        }

    #if defined(SR_RELEASE)
        HMODULE crt = GetModuleHandleA("ucrtbase.dll");
    #else
        HMODULE crt = GetModuleHandleA("ucrtbased.dll");
    #endif
        if (crt) {
            /// malloc/free
            LPVOID mallocAddr = static_cast<LPVOID>(GetProcAddress(crt, "malloc"));
            LPVOID freeAddr   = static_cast<LPVOID>(GetProcAddress(crt, "free"));
            MH_CreateHook(mallocAddr, &MemoryHookDetails::hook_malloc, reinterpret_cast<void**>(&MemoryHookDetails::real_malloc));
            MH_CreateHook(freeAddr, &MemoryHookDetails::hook_free, reinterpret_cast<void**>(&MemoryHookDetails::real_free));
            MH_EnableHook(mallocAddr);
            MH_EnableHook(freeAddr);

            /// VirtualAlloc/Free
            MH_CreateHook(&VirtualAlloc, &MemoryHookDetails::hook_VirtualAlloc, reinterpret_cast<void**>(&MemoryHookDetails::real_VirtualAlloc));
            MH_CreateHook(&VirtualFree, &MemoryHookDetails::hook_VirtualFree, reinterpret_cast<void**>(&MemoryHookDetails::real_VirtualFree));
            MH_EnableHook(&VirtualAlloc);
            MH_EnableHook(&VirtualFree);

            /// HeapAlloc/Free
            MH_CreateHook(&HeapAlloc, &MemoryHookDetails::hook_HeapAlloc, reinterpret_cast<void**>(&MemoryHookDetails::real_HeapAlloc));
            MH_CreateHook(&HeapFree, &MemoryHookDetails::hook_HeapFree, reinterpret_cast<void**>(&MemoryHookDetails::real_HeapFree));
            MH_EnableHook(&HeapAlloc);
            MH_EnableHook(&HeapFree);
        }
        else {
            SR_PLATFORM_NS::WriteConsoleError("InitMemoryHooks() : failed to get CRT module handle!\n");
        }

        SR_PLATFORM_NS::WriteConsoleLog("InitMemoryHooks() : memory hooks initialized.\n");
    }

    void DeInitMemoryHooks() {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
}

#else
namespace SR_UTILS_NS {
    void InitMemoryHooks() { }
    void DeInitMemoryHooks() { }
}
#endif