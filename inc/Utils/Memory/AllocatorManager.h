//
// Created by Monika on 18.07.2026.
//

#ifndef SR_ENGINE_COMMON_ALLOCATOR_MANAGER_H
#define SR_ENGINE_COMMON_ALLOCATOR_MANAGER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Types/Vector.h>

namespace SR_UTILS_NS {
    class AllocatorManager : public Singleton<AllocatorManager> {
        SR_REGISTER_SINGLETON(AllocatorManager)
        struct AllocatorInfo {
            SR_UTILS_NS::StringAtom name;
            SR_HTYPES_NS::RawPointerHolder<IAllocator> pAllocator;
        };
    public:
        SR_NODISCARD IAllocator* GetAllocator(SR_UTILS_NS::StringAtom name) const;
        SR_NODISCARD IAllocator* GetAllocator(SR_UTILS_NS::StringView name) const;
        SR_NODISCARD IAllocator* GetAllocator(const char* name) const;

        void RegisterAllocator(SR_UTILS_NS::StringAtom name, IAllocator* pAllocator);
        void UnregisterAllocator(SR_UTILS_NS::StringAtom name);

    private:
        Vector<AllocatorInfo> m_allocators;
        mutable std::recursive_mutex m_mutex;

    };
}

#endif //SR_ENGINE_COMMON_ALLOCATOR_MANAGER_H
