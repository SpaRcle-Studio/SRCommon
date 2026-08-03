//
// Created by Monika on 28.10.2024.
//

#include <Utils/TypeTraits/Factory.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Debug.h>
#include <Utils/Common/Breakpoint.h>

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

    Vector<SR_UTILS_NS::StringAtom> Factory::GetInheritances(SR_UTILS_NS::StringAtom baseClass) const noexcept {
        Vector<SR_UTILS_NS::StringAtom> result;
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

    SRClass* Factory::CreateBase(SR_UTILS_NS::StringAtom name) const noexcept {
        SR_TRACY_ZONE;

        auto&& pIt = m_types.find(name);

        if (pIt != m_types.end()) {
            if (pIt->second.isAbstract) {
                SR_ERROR("Factory::CreateBase() : type \"{}\" is abstract!", name);
                return nullptr;
            }

            auto&& pClass = pIt->second.allocator();
            if (pClass) {
                return pClass;
            }

            SRHalt("Failed to create object \"{}\"!", name);
            return nullptr;
        }

        SRHalt("Type \"{}\" is not registered!", name);
        return nullptr;
    }

    const SRClassMeta* Factory::GetType(SR_UTILS_NS::StringAtom name) const noexcept {
        auto&& pIt = m_types.find(name);
        if (pIt != m_types.end()) {
            return pIt->second.metaGetter();
        }
        return nullptr;
    }

    void Factory::ForEachClassInModule(SR_UTILS_NS::StringAtom moduleName, const SR_HTYPES_NS::Function<void(const SRClassMeta*)>& func) const noexcept {
        for (auto&& [name, info] : m_types) {
            if (info.moduleName == moduleName) {
                if (auto&& pMeta = info.metaGetter()) {
                    func(pMeta);
                }
            }
        }
    }

    const Factory::TypeInfo* Factory::GetTypeInfo(SR_UTILS_NS::StringAtom name) const noexcept {
        auto&& pIt = m_types.find(name);
        if (pIt != m_types.end()) {
            return &pIt->second;
        }
        return nullptr;
    }

    void Factory::WriteLog(const std::string& message) const noexcept {
        if (SR_UTILS_NS::Debug::IsSingletonInitialized()) {
            if (SR_UTILS_NS::Debug::Instance().IsInitialized()) {
                SR_LOG(message);
                return;
            }
        }
        SR_PLATFORM_NS::WriteConsoleLog(std::string(message).append("\n"));
    }

    void Factory::WriteError(const std::string& message) const noexcept {
        if (SR_UTILS_NS::Debug::IsSingletonInitialized()) {
            if (SR_UTILS_NS::Debug::Instance().IsInitialized()) {
                SRHalt(message);
                return;
            }
        }
        SR_PLATFORM_NS::WriteConsoleError(std::string(message).append("\n"));
        SR_MAKE_BREAKPOINT;
    }

    bool Factory::HasBaseClass(StringAtom name, StringAtom baseClass) const noexcept {
        if (auto&& pIt = m_types.find(name); pIt != m_types.end()) {
            if (auto&& pMeta = pIt->second.metaGetter()) {
                return pMeta->IsInherited(baseClass);
            }
        }
        return false;
    }

    bool BaseFactory::IsRegistered(const SRClassMeta* pMeta) const {
        return pMeta && GetType(pMeta->GetFactoryName()) == pMeta;
    }
}
