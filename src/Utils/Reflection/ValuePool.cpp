//
// Created by Codex on 20.11.2026.
//

#include <Utils/Reflection/ValuePool.h>

namespace SR_UTILS_NS::Reflection {
    ValuePool::ValuePool(const TypeInfo* pType, const SizeType capacity)
        : m_pType(pType)
    {
        m_slots.resize(capacity);
        auto&& lifecycle = m_pType->vtable.lifecycle;
        for (auto&& slot : m_slots) {
            slot.pData = IAllocator::GetDefaultAllocator()->Allocate(lifecycle.size, lifecycle.alignment);
        }
    }

    ValuePool::~ValuePool() {
        auto&& lifecycle = m_pType->vtable.lifecycle;
        for (auto&& slot : m_slots) {
            if (slot.pData) {
                IAllocator::GetDefaultAllocator()->Free(slot.pData, lifecycle.size, lifecycle.alignment);
            }
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
