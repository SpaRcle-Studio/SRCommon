//
// Created by Monika on 18.07.2026.
//

#include <Utils/Memory/AllocatorManager.h>
#include <Utils/Types/LockGuard.h>

namespace SR_UTILS_NS {
    IAllocator* AllocatorManager::GetAllocator(SR_UTILS_NS::StringAtom name) const {
        SR_LOCK_GUARD;
        for (const auto& info : m_allocators) {
            if (info.name == name) {
                return info.pAllocator.Get();
            }
        }
        return nullptr;
    }

    IAllocator* AllocatorManager::GetAllocator(SR_UTILS_NS::StringView name) const {
        SR_LOCK_GUARD;
        for (const auto& info : m_allocators) {
            if (info.name.ToStringView() == name) {
                return info.pAllocator.Get();
            }
        }
        return nullptr;
    }

    IAllocator* AllocatorManager::GetAllocator(const char* name) const {
        return GetAllocator(SR_UTILS_NS::StringView(name));
    }

    void AllocatorManager::RegisterAllocator(SR_UTILS_NS::StringAtom name, IAllocator* pAllocator) {
        SR_LOCK_GUARD;
        for (const auto& info : m_allocators) {
            if (info.name == name) {
                SRHalt("AllocatorManager::RegisterAllocator() : allocator with name \"{}\" is already registered!", name);
                return;
            }
        }
        m_allocators.push_back({name, pAllocator});
    }

    void AllocatorManager::UnregisterAllocator(SR_UTILS_NS::StringAtom name) {
        SR_LOCK_GUARD;
        auto it = std::remove_if(m_allocators.begin(), m_allocators.end(), [&](const AllocatorInfo& info) {
            return info.name == name;
        });
        if (it != m_allocators.end()) {
            m_allocators.erase(it, m_allocators.end());
        }
        else {
            SRHalt("AllocatorManager::UnregisterAllocator() : allocator with name \"{}\" is not registered!", name);
        }
    }
}