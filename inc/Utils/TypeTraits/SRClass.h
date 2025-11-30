//
// Created by Monika on 02.04.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H

#include <Utils/Common/Enumerations.h>
#include <Utils/TypeTraits/SRClassMacro.h>

namespace SR_UTILS_NS {
    class SRClassMeta;

    /// Флаги для сериализатора объектов
    SR_ENUM_NS_STRUCT_T(SerializationFlags, uint64_t,
        None     = 1 << 0,
        Compress = 1 << 1,
        NoUID    = 1 << 2,
        DontSave = 1 << 3
    )

    struct SerializableVerifyContext {
    public:
        void AddError(const std::string& error) noexcept { errors.insert(error); }
        void AddWarning(const std::string& warning) noexcept { warnings.insert(warning); }

        SR_NODISCARD const std::set<std::string>& GetErrors() const noexcept { return errors; }
        SR_NODISCARD const std::set<std::string>& GetWarnings() const noexcept { return warnings; }

    private:
        std::set<std::string> errors;
        std::set<std::string> warnings;
    };

    class SRClass {
    public:
        virtual ~SRClass() = default;

    public:
        SR_NODISCARD static std::span<const SRClassMeta*> GetBaseMetas() noexcept {
            return {};
        }

        SR_NODISCARD virtual const SR_UTILS_NS::SRClassMeta* GetMeta() const noexcept = 0;

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