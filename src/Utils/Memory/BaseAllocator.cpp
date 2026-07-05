//
// Created by Monika on 27.04.2025.
//

#include <Utils/Profile/TracyContext.h>

bool g_TracyAllocatorInitialized = false;

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
    return SRMalloc(size);
}

void operator delete(void* pMemory) noexcept {
    SRFree(pMemory);
}

void* operator new[](SR_UTILS_NS::SizeType size) {
    return SRMalloc(size);
}

void operator delete[](void* pMemory) noexcept {
    SRFree(pMemory);
}