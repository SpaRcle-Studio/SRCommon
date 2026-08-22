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

    void DefaultAllocator::Free(void* pMemory) {
        SRFree(pMemory);
    }

    void DefaultAllocator::ResetMemory() {
        /// Default allocator does not support resetting memory.
    }

    /// ================================================================================================================

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

    void UnSynchronizedPoolAllocator::Free(void* pMemory) {
        SRHalt("UnSynchronizedPoolAllocator::Free() : not available for pool allocator!");
    }

    void UnSynchronizedPoolAllocator::ResetMemory() {
        m_poolResource.release();
    }

    /// ================================================================================================================

    void* MonotonicAllocator::Chunk::TryAllocate(SizeType size, SizeType alignment) {
        for (;;) {
            SizeType current = offset;
            uintptr_t begin = reinterpret_cast<uintptr_t>(memory) + current;
            uintptr_t aligned = (begin + alignment - 1) & ~(alignment - 1);
            SizeType newOffset = (aligned - reinterpret_cast<uintptr_t>(memory)) + size;

            if (newOffset > capacity)
                return nullptr;

            if (current == offset) {
                offset = newOffset;
                return reinterpret_cast<void*>(aligned);
            }
        }
    }

    MonotonicAllocator::MonotonicAllocator(SR_UTILS_NS::SizeType chunkSize)
        : m_chunkSize(chunkSize)
    {
        m_current = CreateChunk(chunkSize);
    }

    MonotonicAllocator::~MonotonicAllocator() {
        SR_TRACY_ZONE;
        Chunk* current = m_current;
        while (current) {
            Chunk* next = current->next;
            SRFree(current->memory);
            current->~Chunk();
            SRFree(current);
            current = next;
        }
    }

    void* MonotonicAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        for (;;) {
            if (void* pMemory = m_current->TryAllocate(size, alignment)) {
                return pMemory;
            }
            Grow(m_current, std::max(m_chunkSize, size + alignment));
        }
    }

    void* MonotonicAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRHalt("MonotonicAllocator::ReAllocate() : not available for monotonic allocator!");
        return nullptr;
    }

    void MonotonicAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        /// Monotonic allocator does not support freeing individual allocations.
    }
    void MonotonicAllocator::Free(void* pMemory) {
        /// Monotonic allocator does not support freeing individual allocations.
    }

    void MonotonicAllocator::Grow(MonotonicAllocator::Chunk* pExpected, SizeType size) {
        if (m_current != pExpected)
            return;
        m_current = CreateChunk(size);
    }

    MonotonicAllocator::Chunk* MonotonicAllocator::CreateChunk(SizeType size) {
        auto&& chunk = (Chunk*)SRMalloc(sizeof(Chunk));
        new (chunk) Chunk();

        chunk->memory = static_cast<uint8_t*>(SRMalloc(size));
        chunk->capacity = size;

        if (!chunk->memory) {
            chunk->~Chunk();
            SRFree(chunk);
            SRHalt("MonotonicAllocator::CreateChunk() : failed to allocate memory for chunk!");
            return nullptr;
        }

        if (!m_head) {
            m_head = chunk;
        }
        else {
            m_tail->next = chunk;
        }

        m_tail = chunk;

        return chunk;
    }

    void MonotonicAllocator::ResetMemory() {
        Chunk* current = m_head;
        while (current) {
            current->offset = 0;
            current = current->next;
        }
    }

    SizeType MonotonicAllocator::GetUsedMemory() const {
        SizeType usedMemory = 0;
        Chunk* current = m_head;
        while (current) {
            usedMemory += current->offset;
            current = current->next;
        }
        return usedMemory;
    }

    /// ================================================================================================================

    void* SyncMonotonicAllocator::Chunk::TryAllocate(size_t size, size_t alignment) {
        for (;;) {
            size_t current = offset.load(std::memory_order_relaxed);
            uintptr_t begin = reinterpret_cast<uintptr_t>(memory) + current;
            uintptr_t aligned = (begin + alignment - 1) & ~(alignment - 1);
            size_t newOffset = (aligned - reinterpret_cast<uintptr_t>(memory)) + size;

            if (newOffset > capacity)
                return nullptr;

            if (offset.compare_exchange_weak(current, newOffset, std::memory_order_relaxed)) {
                return reinterpret_cast<void*>(aligned);
            }
        }
    }

    SyncMonotonicAllocator::SyncMonotonicAllocator(SR_UTILS_NS::SizeType chunkSize)
        : m_chunkSize(chunkSize)
    {
        m_current.store(CreateChunk(chunkSize));
    }

    SyncMonotonicAllocator::~SyncMonotonicAllocator() {
        Chunk* current = m_current.load();
        while (current) {
            Chunk* next = current->next;
            SRFree(current->memory);
            current->~Chunk();
            SRFree(current);
            current = next;
        }
    }

    void* SyncMonotonicAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        for (;;)
        {
            Chunk* chunk = m_current.load(std::memory_order_acquire);
            if (void* ptr = chunk->TryAllocate(size, alignment))
                return ptr;
            Grow(chunk, std::max(m_chunkSize, size + alignment));
        }
    }

    void* SyncMonotonicAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRHalt("SyncMonotonicAllocator::ReAllocate() : not available for monotonic allocator!");
        return nullptr;
    }

    void SyncMonotonicAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        /// Monotonic allocator does not support freeing individual allocations.
    }
    void SyncMonotonicAllocator::Free(void* pMemory) {
        /// Monotonic allocator does not support freeing individual allocations.
    }

    void SyncMonotonicAllocator::Grow(SyncMonotonicAllocator::Chunk* pExpected, SizeType size) {
        std::lock_guard lock(m_mutex);
        if (m_current.load(std::memory_order_relaxed) != pExpected)
            return;
        Chunk* chunk = CreateChunk(size);
        m_current.store(chunk, std::memory_order_release);
    }

    SyncMonotonicAllocator::Chunk* SyncMonotonicAllocator::CreateChunk(SizeType size) {
        auto&& chunk = (Chunk*)SRMalloc(sizeof(Chunk));
        new (chunk) Chunk();

        chunk->memory = static_cast<uint8_t*>(SRMalloc(size));
        chunk->capacity = size;

        if (!chunk->memory) {
            chunk->~Chunk();
            SRFree(chunk);
            SRHalt("SyncMonotonicAllocator::CreateChunk() : failed to allocate memory for chunk!");
            return nullptr;
        }

        if (!m_head) {
            m_head = chunk;
        }
        else {
            m_tail->next = chunk;
        }

        m_tail = chunk;

        return chunk;
    }

    void SyncMonotonicAllocator::ResetMemory() {
        std::lock_guard lock(m_mutex);
        Chunk* current = m_head;
        while (current) {
            current->offset.store(0, std::memory_order_relaxed);
            current = current->next;
        }
    }

    /// ================================================================================================================

    void* SynchronizedPoolAllocator::Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        return m_poolResource.allocate(size, alignment);
    }

    void* SynchronizedPoolAllocator::ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        SRHalt("SynchronizedPoolAllocator::ReAllocate() : not available for pool allocator!");
        return nullptr;
    }

    void SynchronizedPoolAllocator::Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) {
        if (m_deallocEnabled) {
            m_poolResource.deallocate(pMemory, size, alignment);
        }
    }

    void SynchronizedPoolAllocator::Free(void* pMemory) {
        if (m_deallocEnabled) {
            SRHalt("SynchronizedPoolAllocator::Free() : not available for pool allocator!");
        }
    }

    SynchronizedPoolAllocator::SynchronizedPoolAllocator(bool deallocEnabled)
        : m_deallocEnabled(deallocEnabled)
    { }

    void SynchronizedPoolAllocator::ResetMemory() {
        m_poolResource.release();
    }
}