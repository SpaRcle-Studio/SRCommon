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
        m_slots.resize(capacity);
        auto&& lifecycle = m_pType->vtable.lifecycle;
        const SizeType stride = ((lifecycle.size + lifecycle.alignment - 1) / lifecycle.alignment) * lifecycle.alignment;
        m_storageSize = stride * capacity;
        if (m_storageSize > 0) {
            m_pStorage = IAllocator::GetDefaultAllocator()->Allocate(m_storageSize, lifecycle.alignment);
            for (SizeType i = 0; i < capacity; ++i) {
                m_slots[i].pData = static_cast<uint8_t*>(m_pStorage) + i * stride;
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
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto&& slot : m_slots) {
            if (!slot.used && slot.pData) {
                m_pType->ConstructAt(slot.pData);
                slot.used = true;
                ++m_references;
                Value result;
                result.m_typeInfo = m_pType;
                result.m_storage = ReflectedValue::MakeFromPointer(slot.pData, ReflectedValueStorageType::Dynamic);
                result.m_pPool = this;
                return result;
            }
        }
        return {};
    }

    void ValuePool::Release(void* pData) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto&& slot : m_slots) {
                if (slot.pData == pData) {
                    slot.used = false;
                    break;
                }
            }
        }
        ReleaseOwner();
    }

    void ValuePool::ReleaseOwner() {
        if (--m_references == 0) {
            delete this;
        }
    }
}
