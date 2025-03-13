//
// Created by Monika on 28.10.2024.
//

#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS {
    Factory& Factory::Instance() noexcept {
        static std::atomic<Factory*> pInstance = nullptr;
        Factory* pTmp = pInstance.load(std::memory_order_acquire);
        if (pTmp == nullptr) {
            auto&& pNewInstance = new Factory();
            if (!pInstance.compare_exchange_strong(pTmp, pNewInstance, std::memory_order_release, std::memory_order_relaxed)) {
                delete pNewInstance;
            } else {
                pTmp = pNewInstance;
            }
        }
        return *pTmp;
    }

    SR_UTILS_NS::StringAtom Factory::GetName(const SRClassMeta* pMeta, const bool isMustExists) const {
        if (IsRegistered(pMeta)) {
            return pMeta->GetFactoryName(); /// NOLINT
        }

        if (isMustExists) {
            if (!pMeta) {
                SRHalt("Factory::GetName() : meta is nullptr!");
            }
            else {
                SRHalt("Factory::GetName() : meta is not registered!");
            }
        }
        return {};
    }

    std::vector<SR_UTILS_NS::StringAtom> Factory::GetInheritances(SR_UTILS_NS::StringAtom baseClass) const noexcept {
        std::vector<SR_UTILS_NS::StringAtom> result;
        for (auto&& [name, info] : m_types) {
            if (auto&& pMeta = info.metaGetter()) {
                if (pMeta->IsInherited(baseClass)) {
                    result.push_back(name);
                }
            }
        }
        return result;
    }

    bool Factory::IsAbstract(SR_UTILS_NS::StringAtom name) const noexcept {
        if (auto&& pIt = m_types.find(name); pIt != m_types.end()) {
            return pIt->second.isAbstract;
        }
        SRHalt("Factory::IsAbstract() : unknown type! Name: {}", name);
        return false;
    }
}
