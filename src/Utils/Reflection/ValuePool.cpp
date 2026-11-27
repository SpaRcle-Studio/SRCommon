//
// Created by Codex on 20.11.2026.
//

#include <Utils/Reflection/ValuePool.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    ValuePool::ValuePool(const TypeInfo* pType, const SizeType capacity)
        : m_pType(pType)
    {
        SR_TRACY_ZONE;
        auto&& lifecycle = m_pType->vtable.lifecycle;
        const SizeType stride = ((lifecycle.size + lifecycle.alignment - 1) / lifecycle.alignment) * lifecycle.alignment;
        m_storageSize = stride * capacity;
        if (m_storageSize > 0) {
            m_pStorage = IAllocator::GetDefaultAllocator()->Allocate(m_storageSize, lifecycle.alignment);
            m_freeSlots.reserve(capacity);

            /// первый слот оказывается на вершине стека
            for (SizeType i = capacity; i > 0; --i) {
                m_freeSlots.emplace_back(static_cast<uint8_t*>(m_pStorage) + (i - 1) * stride);
            }
        }
    }

    ValuePool::~ValuePool() {
        auto&& lifecycle = m_pType->vtable.lifecycle;
        if (m_pStorage) {
            IAllocator::GetDefaultAllocator()->Free(m_pStorage, m_storageSize, lifecycle.alignment);
        }
    }

    Value ValuePool::Acquire() {
        void* pData = nullptr;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_freeSlots.empty()) {
                return {};
            }

            pData = m_freeSlots.back();
            m_freeSlots.pop_back();
        }

        m_pType->ConstructAt(pData);
        ++m_references;

        Value result;
        result.m_typeInfo = m_pType;
        result.m_storage = ReflectedValue::MakeFromPointer(pData, ReflectedValueStorageType::Dynamic);
        result.m_pPool = this;
        return result;
    }

    void ValuePool::Release(void* pData) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_freeSlots.emplace_back(pData);
        }
        ReleaseOwner();
    }

    void ValuePool::ReleaseOwner() {
        if (--m_references == 0) {
            delete this;
        }
    }
}
