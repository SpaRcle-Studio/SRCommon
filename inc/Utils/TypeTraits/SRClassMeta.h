//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H
#define SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H

#include <Utils/Types/StringAtom.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>

namespace SR_UTILS_NS {
    class Serializable;

    class SRClassMeta {
    public:
        virtual ~SRClassMeta() = default;

        virtual void Save(SR_UTILS_NS::ISerializer& serializer, const SR_UTILS_NS::Serializable& obj) const;
        virtual void Load(SR_UTILS_NS::IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj) const;

        SR_NODISCARD virtual bool IsAbstract() const noexcept { return false; }
        SR_NODISCARD virtual bool IsEditorOnly() const noexcept { return false; }
        SR_NODISCARD virtual std::span<const SRClassMeta*> GetBaseMetas() const noexcept { return {}; }
        SR_NODISCARD virtual std::string_view GetFactoryName() const noexcept { return {}; }
    };
};

namespace Codegen {
    template<class T> struct SRClassMetaTemplate : public SR_UTILS_NS::SRClassMeta { };
}


#define SR_CLASS()                                                                                                      \
    public:                                                                                                             \
        static bool RegisterPropertiesCodegen();                                                                        \
    private:                                                                                                            \
        template<typename TSerializeType> friend struct ::Codegen::SRClassMetaTemplate;                                 \
        friend class SR_UTILS_NS::SRClass;                                                                              \
                                                                                                                        \
    public:                                                                                                             \
        static const SR_UTILS_NS::SRClassMeta* GetMetaStatic() noexcept;                                                \
        virtual const SR_UTILS_NS::SRClassMeta* GetMeta() const noexcept {                                              \
            auto&& pStaticMetaFromClass = GetMetaStatic();                                                              \
            SRAssert2(pStaticMetaFromClass, std::string("No static meta for: ") + typeid(*this).name());                \
            return pStaticMetaFromClass;                                                                                \
        }                                                                                                               \
                                                                                                                        \
        static SR_UTILS_NS::StringAtom GetClassStaticName() noexcept;                                                   \
    private:


#define SR_CLASS_REGISTER_PROPERTY_BASE(className, propertyName, propertyType)                                          \

#endif //SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H
