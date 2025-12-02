//
// Created by Monika on 27.04.2025.
//

#include <Utils/stdInclude.h>

void* SRMalloc(SR_UTILS_NS::SizeType size) {
    return std::malloc(size);
}

void* SRReAlloc(void* pMemory, SR_UTILS_NS::SizeType size) {
    return std::realloc(pMemory, size);
}

void SRFree(void* pMemory) {
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