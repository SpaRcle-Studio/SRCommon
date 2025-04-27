//
// Created by Monika on 27.04.2025.
//

#ifndef SR_ENGINE_UTILS_MEMORY_ALLOCATOR_H
#define SR_ENGINE_UTILS_MEMORY_ALLOCATOR_H

void* SRMalloc(SR_UTILS_NS::SizeType size);
void* SRReAlloc(void* pMemory, SR_UTILS_NS::SizeType size);
void SRFree(void* pMemory);

template<typename T, typename... Args> T* SRNew(Args&& ...args) {
    void* pMemory = SRMalloc(sizeof(T));
    T* pObject = new(pMemory) T(Forward<Args>(args)...);
    if constexpr (requires(T t) { t.InitializeClass(); }) {
        pObject->InitializeClass();
    }
    return pObject;
}

template<typename T> void SRDelete(T* pObject) {
    if constexpr (requires(T t) { t.DeinitializeClass(); }) {
        pObject->DeinitializeClass();
    }
    pObject->~T();
    SRFree(pObject);
}

void* operator new(SR_UTILS_NS::SizeType size);
void operator delete(void* pMemory) noexcept;
void* operator new[](SR_UTILS_NS::SizeType size);
void operator delete[](void* pMemory) noexcept;

#else
    #error "Allocator.h already included!"
#endif //SR_ENGINE_UTILS_MEMORY_ALLOCATOR_H
