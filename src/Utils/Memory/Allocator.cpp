//
// Created by Monika on 04.07.2026.
//

#include <Utils/Memory/Allocator.h>

namespace SR_UTILS_NS {
    void* DefaultAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        return SRMalloc(size);
    }

    void* DefaultAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        return SRReAlloc(pMemory, size);
    }

    void DefaultAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRFree(pMemory);
    }

    void* UnSynchronizedPoolAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        return m_poolResource.allocate(size, alignment);
    }

    void* UnSynchronizedPoolAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRHalt("UnSynchronizedPoolAllocator::ReAllocate() : not available for pool allocator!");
        return nullptr;
    }

    void UnSynchronizedPoolAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        m_poolResource.deallocate(pMemory, size, alignment);
    }

    MonotonicAllocator::MonotonicAllocator(SR_UTILS_NS::SizeType size)
        : m_monotonicResource(size)
    { }

    void* MonotonicAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        return m_monotonicResource.allocate(size, alignment);
    }

    void* MonotonicAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRHalt("MonotonicAllocator::ReAllocate() : not available for monotonic allocator!");
        return nullptr;
    }

    void MonotonicAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        /// Monotonic allocator does not support freeing individual allocations.
    }
}