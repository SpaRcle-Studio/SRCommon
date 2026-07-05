//
// Created by Monika on 04.07.2026.
//

#ifndef SR_ENGINE_COMMON_MEMORY_ALLOCATOR_H
#define SR_ENGINE_COMMON_MEMORY_ALLOCATOR_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API DefaultAllocator : public IAllocator {
    public:
        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;

    };

    class SR_COMMON_DLL_API UnSynchronizedPoolAllocator : public IAllocator {
    public:
        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;

    private:
        std::pmr::unsynchronized_pool_resource m_poolResource;

    };

    class SR_COMMON_DLL_API MonotonicAllocator : public IAllocator {
    public:
        explicit MonotonicAllocator(SR_UTILS_NS::SizeType blockSize = 1024 * 1024);

        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;

    private:
        std::pmr::monotonic_buffer_resource m_monotonicResource;

    };
}

#endif //SR_ENGINE_COMMON_MEMORY_ALLOCATOR_H
