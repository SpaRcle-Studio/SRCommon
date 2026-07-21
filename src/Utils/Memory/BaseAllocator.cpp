//
// Created by Monika on 27.04.2025.
//

#include <Utils/Memory/AllocatorManager.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Profile/TracyContext.h>

bool g_TracyAllocatorInitialized = false;
SR_THREAD_LOCAL SR_UTILS_NS::IAllocator* g_ThreadLocalAllocator = nullptr;

namespace SR_UTILS_NS {
    static std::atomic<int64_t> g_heapAllocatedBytes = 0;

    void OnMemoryAllocated(SR_UTILS_NS::SizeType size) {
        g_heapAllocatedBytes += size;
    }

    void OnMemoryFreed(SR_UTILS_NS::SizeType size) {
        g_heapAllocatedBytes -= size;
    }

    SR_UTILS_NS::SizeType GetApplicationHeapSize() {
        return g_heapAllocatedBytes.load();
    }

    void SetThreadLocalAllocator(SR_UTILS_NS::IAllocator* pAllocator) {
        g_ThreadLocalAllocator = pAllocator;
    }

    SR_UTILS_NS::IAllocator* GetThreadLocalAllocator() {
        return g_ThreadLocalAllocator;
    }

    IAllocator* IAllocator::GetDefaultAllocator() {
        const static SR_UTILS_NS::StringAtom defaultAllocatorName = "DefaultAllocator";
        auto&& manager = AllocatorManager::Instance();
        if (auto&& pAllocator = manager.GetAllocator(defaultAllocatorName)) {
            return pAllocator;
        }
        manager.RegisterAllocator(defaultAllocatorName, new DefaultAllocator());
        return manager.GetAllocator(defaultAllocatorName);
    }
}

void* SRMalloc(SR_UTILS_NS::SizeType size) {
    if (g_TracyAllocatorInitialized) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_COLOR(0xFF00FF00);
        void* pAllocation = std::malloc(size);
        if (!pAllocation) {
            SRHalt("SRMalloc() : failed to allocate memory! Size: {}Mb", size / (1024 * 1024));
        }
        SR_TRACY_ALLOC(pAllocation, size);
        return pAllocation;
    }
    void* pAllocation = std::malloc(size);
    if (!pAllocation) {
        SRHalt("SRMalloc() : failed to allocate memory! Size: {}Mb", size / (1024 * 1024));
    }
    return pAllocation;
}

void* SRReAlloc(void* pMemory, SR_UTILS_NS::SizeType size) {
    if (g_TracyAllocatorInitialized) {
        SR_TRACY_ZONE;
        return std::realloc(pMemory, size);
    }
    return std::realloc(pMemory, size);
}

void SRFree(void* pMemory) {
    if (g_TracyAllocatorInitialized) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_COLOR(0xFFFF0000);
        SR_TRACY_FREE(pMemory);
        return std::free(pMemory);
    }
    std::free(pMemory);
}

void* operator new(SR_UTILS_NS::SizeType size) {
    if (g_ThreadLocalAllocator) {
        return g_ThreadLocalAllocator->Allocate(size);
    }
    return SRMalloc(size);
}

void operator delete(void* pMemory) noexcept {
    if (g_ThreadLocalAllocator) {
        return g_ThreadLocalAllocator->Free(pMemory);
    }
    SRFree(pMemory);
}

void* operator new[](SR_UTILS_NS::SizeType size) {
    if (g_ThreadLocalAllocator) {
        return g_ThreadLocalAllocator->Allocate(size);
    }
    return SRMalloc(size);
}

void operator delete[](void* pMemory) noexcept {
    if (g_ThreadLocalAllocator) {
        return g_ThreadLocalAllocator->Free(pMemory);
    }
    SRFree(pMemory);
}