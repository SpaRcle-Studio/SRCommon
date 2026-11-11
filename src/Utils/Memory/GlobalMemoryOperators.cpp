#include <Utils/Memory/GlobalMemoryOperators.h>

// Link these replacements into each shared engine module so its allocation and
// deallocation calls use the same allocator as Utils.
void* operator new(SR_UTILS_NS::SizeType size) {
    if (auto* pAllocator = SR_UTILS_NS::GetThreadLocalAllocator()) {
        return pAllocator->Allocate(size);
    }
    return SRMalloc(size);
}

void operator delete(void* pMemory) noexcept {
    if (auto* pAllocator = SR_UTILS_NS::GetThreadLocalAllocator()) {
        return pAllocator->Free(pMemory);
    }
    SRFree(pMemory);
}

void operator delete(void* pMemory, SR_UTILS_NS::SizeType) noexcept {
    ::operator delete(pMemory);
}

void* operator new[](SR_UTILS_NS::SizeType size) {
    if (auto* pAllocator = SR_UTILS_NS::GetThreadLocalAllocator()) {
        return pAllocator->Allocate(size);
    }
    return SRMalloc(size);
}

void operator delete[](void* pMemory) noexcept {
    if (auto* pAllocator = SR_UTILS_NS::GetThreadLocalAllocator()) {
        return pAllocator->Free(pMemory);
    }
    SRFree(pMemory);
}

void operator delete[](void* pMemory, SR_UTILS_NS::SizeType) noexcept {
    ::operator delete[](pMemory);
}
