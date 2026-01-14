//
// Created by Monika on 27.04.2025.
//

#include <Utils/stdInclude.h>
#include <Utils/Profile/TracyContext.h>

bool g_TracyAllocatorInitialized = false;

void* SRMalloc(SR_UTILS_NS::SizeType size) {
    if (g_TracyAllocatorInitialized) {
        SR_TRACY_ZONE;
        void* pMemory = std::malloc(size);
        TracyAlloc(pMemory, size);
        return pMemory;
    }
    return std::malloc(size);
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
        TracyFree(pMemory);
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