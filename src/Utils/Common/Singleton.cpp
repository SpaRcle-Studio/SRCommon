//
// Created by Monika on 15.06.2022.
//

#include <Utils/Common/Singleton.h>

namespace SR_UTILS_NS {
    SingletonManager* GetSingletonManager() noexcept {
        static std::atomic<SingletonManager*> pSingletonManager {nullptr};

        SingletonManager* pLocalPtr = pSingletonManager.load(std::memory_order_acquire);
        if (!pLocalPtr) {
            auto&& newManager = new SingletonManager();

            if (pSingletonManager.compare_exchange_strong(pLocalPtr, newManager, std::memory_order_release)) {
                /// Successfully set the singleton manager
                pLocalPtr = newManager;
            }
            else {
                /// Failed to set the singleton manager, another thread did it
                delete newManager;
            }
        }

        return pLocalPtr;
    }

    void* SingletonManager::GetSingleton(StringAtom name) noexcept {
        std::lock_guard lock(m_mutex);

        if (auto&& pIt = m_singletons.find(name); pIt != m_singletons.end()) {
            return pIt->second.pSingleton;
        }

        return nullptr;
    }

    void SingletonManager::DestroyAll() {
        std::lock_guard lock(m_mutex);

        for (auto pIt = m_singletons.begin(); pIt != m_singletons.end(); ) {
            auto&& [id, info] = *pIt;

            if (info.pSingletonBase->IsSingletonCanBeDestroyed()) {
                info.pSingletonBase->OnSingletonDestroy();
                delete info.pSingletonBase;
                pIt = m_singletons.erase(pIt);
            }
            else {
                ++pIt;
            }
        }
    }

    void SingletonManager::Remove(StringAtom name) {
        std::lock_guard lock(m_mutex);

        if (auto&& pIt = m_singletons.find(name); pIt != m_singletons.end()) {
            m_singletons.erase(pIt);
        }
    }

    std::recursive_mutex& SingletonManager::GetCreationMutex(StringAtom name) {
        std::lock_guard lock(m_mutex);
        std::recursive_mutex& mutex = m_creationMutexes[name];
        return mutex;
    }
}