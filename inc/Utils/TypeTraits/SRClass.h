//
// Created by Monika on 02.04.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Types/Marshal.h>
#include <Utils/TypeTraits/ClassDB.h>
#include <Utils/TypeTraits/SRClassMeta.h>

namespace SR_UTILS_NS {
    class SRClassMeta;

    struct PropertyInfo {
        StringAtom name;
        StringAtom className;

        bool operator<(const PropertyInfo& p_info) const {
            return name < p_info.name;
        }
    };

    class SRClass {
    public:
        virtual ~SRClass() = default;

    public:
        static std::span<const SRClassMeta*> GetBaseMetas() noexcept {
            return {};
        }

        virtual void InitializeClass() noexcept { }

        static SR_UTILS_NS::StringAtom GetClassStaticName() noexcept;
        static const SR_UTILS_NS::SRClassMeta* GetMetaStatic() noexcept;
        static bool RegisterPropertiesCodegen();

    private:
        void SR_CLANG_CODEGEN_MARKER() { }
        inline static const bool SR_CODEGEN_INITIALIZE = SR_UTILS_NS::ClassDB::Instance().RegisterNewClass(GetClassStaticName());

    };

    template<class T> static T* PostAllocationInitialize(T* pObject) {
        pObject->InitializeClass();
        return pObject;
    }
}

template<class T, typename ...Args> static T* SRNew(Args&& ...args) {
    return PostAllocationInitialize(new T(std::forward<Args>(args)...));
}

#endif //SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H