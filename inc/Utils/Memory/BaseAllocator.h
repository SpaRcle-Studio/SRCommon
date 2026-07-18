//
// Created by Monika on 27.04.2025.
//

#ifndef SR_ALLOCATOR_INCLUDE_GUARD
    #error "Do not include Allocator.h directly!"
#endif

SR_COMMON_DLL_API extern bool g_TracyAllocatorInitialized;

namespace SR_UTILS_NS {
    class IAllocator;

    SR_COMMON_DLL_API void SetThreadLocalAllocator(SR_UTILS_NS::IAllocator* pAllocator);
    SR_COMMON_DLL_API SR_UTILS_NS::IAllocator* GetThreadLocalAllocator();

    SR_COMMON_DLL_API extern void OnMemoryAllocated(SR_UTILS_NS::SizeType size);
    SR_COMMON_DLL_API extern void OnMemoryFreed(SR_UTILS_NS::SizeType size);
    SR_COMMON_DLL_API extern SR_UTILS_NS::SizeType GetApplicationHeapSize();

    class SR_COMMON_DLL_API IAllocator {
    public:
        virtual ~IAllocator() = default;

        SR_NODISCARD virtual void* Allocate(
            SR_UTILS_NS::SizeType size,
            SR_UTILS_NS::SizeType alignment = alignof(MaxAlignT)
        ) = 0;

        SR_NODISCARD virtual void* ReAllocate(
            void* pMemory,
            SR_UTILS_NS::SizeType size,
            SR_UTILS_NS::SizeType alignment = alignof(MaxAlignT)
        ) = 0;

        virtual void Free(
            void* pMemory,
            SR_UTILS_NS::SizeType size,
            SR_UTILS_NS::SizeType alignment = alignof(MaxAlignT)
        ) = 0;

        virtual void Free(void* pMemory) = 0;

    };
}

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

