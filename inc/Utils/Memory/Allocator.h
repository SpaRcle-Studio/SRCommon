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
        void Free(void* pMemory) override;
        void ResetMemory() override;

    };

    class SR_COMMON_DLL_API UnSynchronizedPoolAllocator : public IAllocator {
    public:
        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory) override;
        void ResetMemory() override;

    private:
        std::pmr::unsynchronized_pool_resource m_poolResource;

    };

    class SR_COMMON_DLL_API MonotonicAllocator : public IAllocator {
        struct Chunk {
            uint8_t* memory = nullptr;
            SizeType capacity = 0;
            SizeType offset = 0;
            Chunk* next = nullptr;
            void* TryAllocate(size_t size, size_t alignment);
        };
    public:
        explicit MonotonicAllocator(SR_UTILS_NS::SizeType chunkSize = 1024 * 1024);
        ~MonotonicAllocator() override;

        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory) override;
        void ResetMemory() override;

    private:
        void Grow(Chunk* pExpected, SizeType size);
        SR_NODISCARD Chunk* CreateChunk(SizeType size);

    private:
        Chunk* m_head = nullptr;
        Chunk* m_tail = nullptr;
        Chunk* m_current = nullptr;
        SizeType m_chunkSize = 0;

    };

    class SR_COMMON_DLL_API SyncMonotonicAllocator : public IAllocator {
        struct Chunk {
            uint8_t* memory = nullptr;
            SizeType capacity = 0;
            std::atomic<SizeType> offset = 0;
            Chunk* next = nullptr;
            void* TryAllocate(size_t size, size_t alignment);
        };
    public:
        explicit SyncMonotonicAllocator(SR_UTILS_NS::SizeType chunkSize = 1024 * 1024);
        ~SyncMonotonicAllocator() override;

        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory) override;
        void ResetMemory() override;

    private:
        void Grow(Chunk* pExpected, SizeType size);
        SR_NODISCARD Chunk* CreateChunk(SizeType size);

    private:
        Chunk* m_head = nullptr;
        Chunk* m_tail = nullptr;
        std::atomic<Chunk*> m_current = nullptr;
        std::mutex m_mutex;
        SizeType m_chunkSize = 0;

    };

    class SR_COMMON_DLL_API SynchronizedPoolAllocator : public IAllocator {
    public:
        explicit SynchronizedPoolAllocator(bool deallocEnabled = true);

        SR_NODISCARD void* Allocate(SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        SR_NODISCARD void* ReAllocate(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory, SR_UTILS_NS::SizeType size, SR_UTILS_NS::SizeType alignment) override;
        void Free(void* pMemory) override;
        void ResetMemory() override;

    private:
        std::pmr::synchronized_pool_resource m_poolResource;
        bool m_deallocEnabled = true;

    };
}

#endif //SR_ENGINE_COMMON_MEMORY_ALLOCATOR_H
