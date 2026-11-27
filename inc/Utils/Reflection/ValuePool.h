//
// Created by Codex on 20.11.2026.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_VALUE_POOL_H
#define SR_ENGINE_UTILS_REFLECTION_VALUE_POOL_H

#include <Utils/Reflection/Value.h>

namespace SR_UTILS_NS::Reflection {
    /// Fixed storage for outstanding call results. An acquired Value owns its slot.
    /// The pool survives its caller until all outstanding Values are destroyed.
    class SR_COMMON_DLL_API ValuePool {
    public:
        ValuePool(const TypeInfo* pType, SizeType capacity);
        ValuePool(const ValuePool&) = delete;
        ValuePool& operator=(const ValuePool&) = delete;
        SR_NODISCARD Value Acquire();
        void ReleaseOwner();
        void Release(void* pData);
    private:
        ~ValuePool();
        struct Slot {
            void* pData = nullptr;
            bool used = false;
        };
        const TypeInfo* m_pType = nullptr;
        Vector<Slot> m_slots;
        void* m_pStorage = nullptr;
        SizeType m_storageSize = 0;
        std::atomic<SizeType> m_references = 1;
        std::mutex m_mutex;
    };
}

#endif
