//
// Created by Monika on 02.04.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H

#include <Utils/Common/Enumerations.h>
#include <Utils/TypeTraits/SRClassMacro.h>
#include <Utils/Types/Set.h>

namespace SR_UTILS_NS {
    class SRClassMeta;

    struct SerializableVerifyContext {
    public:
        void AddError(StringView error) noexcept { errors.insert(std::move(String(error))); }
        void AddWarning(StringView warning) noexcept { warnings.insert(std::move(String(warning))); }

        SR_NODISCARD const Set<String>& GetErrors() const noexcept { return errors; }
        SR_NODISCARD const Set<String>& GetWarnings() const noexcept { return warnings; }

    private:
        Set<String> errors;
        Set<String> warnings;
    };

    class SR_COMMON_DLL_API SRClass {
    public:
        virtual ~SRClass() = default;

    public:
        SR_NODISCARD static std::span<const SRClassMeta*> GetBaseMetas() noexcept {
            return {};
        }

        SR_NODISCARD virtual const SR_UTILS_NS::SRClassMeta* GetMeta() const noexcept {
            SRHalt("Abstract method called!");
            return nullptr;
        }

        //virtual void InitializeClass() noexcept { }

        virtual void CloneTo(SRClass& clone) const;

        static SR_UTILS_NS::StringAtom GetClassStaticName() noexcept;
        static const SR_UTILS_NS::SRClassMeta* GetMetaStatic() noexcept;
        static SR_UTILS_NS::SRClass* AllocateStatic() noexcept;

    };

    template<typename T>
    constexpr bool IsSRClassV = std::is_base_of_v<SRClass, RemoveQualifiersT<T>> || std::is_same_v<SRClass, RemoveQualifiersT<T>>;

    //template<class T> static T* PostAllocationInitialize(T* pObject) {
    //    if constexpr (std::is_base_of_v<SRClass, T>) {
    //        pObject->InitializeClass();
    //    }
    //    return pObject;
    //}
}

//template<class T, typename ...Args> static T* SRNew(Args&& ...args) {
//     return PostAllocationInitialize(new T(std::forward<Args>(args)...));
//}

#endif //SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H