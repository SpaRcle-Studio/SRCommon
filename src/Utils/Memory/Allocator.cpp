//
// Created by Monika on 27.04.2025.
//

#include <Utils/stdInclude.h>
#include <Utils/Profile/TracyContext.h>

bool g_TracyAllocatorInitialized = false;

void* SRMalloc(SR_UTILS_NS::SizeType size) {
    void* pMemory = std::malloc(size);
    if (g_TracyAllocatorInitialized) {
        TracyAlloc(pMemory, size);
    }
    return pMemory;
}

void* SRReAlloc(void* pMemory, SR_UTILS_NS::SizeType size) {
    return std::realloc(pMemory, size);
}

void SRFree(void* pMemory) {
    if (g_TracyAllocatorInitialized) {
        TracyFree(pMemory);
    }
    return std::free(pMemory);
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