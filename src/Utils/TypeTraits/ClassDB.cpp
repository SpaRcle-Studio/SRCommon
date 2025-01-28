//
// Created by Monika on 09.10.2024.
//

#include <Utils/TypeTraits/ClassDB.h>

namespace SR_UTILS_NS {
    ClassDB& ClassDB::Instance() {
        static std::atomic<ClassDB*> pInstance = nullptr;
        ClassDB* pTmp = pInstance.load(std::memory_order_acquire);
        if (pTmp == nullptr) {
            auto&& pNewInstance = new ClassDB();
            if (!pInstance.compare_exchange_strong(pTmp, pNewInstance, std::memory_order_release, std::memory_order_relaxed)) {
                delete pNewInstance;
            } else {
                pTmp = pNewInstance;
            }
        }
        return *pTmp;
    }

    void ClassDB::ResolveInheritance() {
        SR_TRACY_ZONE;
        SR_WRITE_LOCK;

        if (m_inheritanceResolved) {
            return;
        }

        for (auto& [name, info] : m_classes) {
            if (!info.inherits.Empty()) {
                auto pIt = m_classes.find(info.inherits);
                if (pIt == m_classes.end()) {
                    SRHalt("ClassDB::ResolveInheritance() : class '" + info.name.ToStringRef() + "' inherits from unknown class '" + info.inherits.ToStringRef() + "'!");
                    continue;
                }

                info.pInherits = &pIt->second;
            }
        }

        m_inheritanceResolved = true;
    }

    bool ClassDB::RegisterNewClass(StringAtom className) {
        SR_WRITE_LOCK;

        if (m_classes.count(className) > 0) {
            SRHalt("ClassDB::RegisterNewClass() : class '" + className.ToStringRef() + "' already exists!");
            return false;
        }

        ClassInfo& info = m_classes.insert(std::make_pair(className, ClassInfo())).first->second;
        info.name = className;
        m_inheritanceResolved = false;
        return true;
    }

    bool ClassDB::RegisterInheritance(StringAtom className, StringAtom inherits) {
        if (className == inherits) {
            SRHalt("ClassDB::RegisterInheritance() : class '" + className.ToStringRef() + "' can't inherit from itself!");
            return false;
        }

        if (m_inheritance.count(className) > 0) {
            SRHalt("ClassDB::RegisterInheritance() : class '" + className.ToStringRef() + "' already has inheritance!");
            return false;
        }

        m_inheritance[className].emplace_back(inherits);
        return true;
    }
}
