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

    std::string_view Factory::GetName(const SRClassMeta* pMeta, const bool isMustExists) const {
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
}