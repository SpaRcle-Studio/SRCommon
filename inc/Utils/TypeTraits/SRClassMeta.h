//
// Created by Monika on 16.10.2024.
//

#if !defined(SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H

#include <Utils/Types/Function.h>
#include <Utils/Reflection/VTable.h>

namespace SR_UTILS_NS {
    class SRClass;
    class StringAtom;
    class ISerializer;
    class IDeserializer;

    namespace Reflection {
        class Property;
        class Method;
    }

    class Serializable;

    class SR_COMMON_DLL_API SRClassMeta {
    public:
        virtual ~SRClassMeta();

        virtual void Save(SR_UTILS_NS::ISerializer& serializer, const SR_UTILS_NS::Serializable& obj) const;
        virtual bool Load(SR_UTILS_NS::IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj) const;

        SR_NODISCARD bool IsInherited(SR_UTILS_NS::StringAtom baseClass) const noexcept;

        void ForEachProperty(const SR_HTYPES_NS::Function<void(const SR_UTILS_NS::Reflection::Property& property, uint64_t index)>& func, uint64_t* pIndex = nullptr) const;
        void ForEachMethod(const SR_HTYPES_NS::Function<void(const SR_UTILS_NS::Reflection::Method& method, uint64_t index)>& func, uint64_t* pIndex = nullptr) const;
        virtual void ForEachSRClass(SRClass& srClass, const SR_HTYPES_NS::Function<void(SRClass&)>& function) const noexcept;
        virtual void CloneTo(const SRClass& src, SRClass& dest) const noexcept;
        virtual Reflection::Value Cast(const Reflection::Value& value) const noexcept;

        SR_NODISCARD virtual SR_UTILS_NS::StringAtom GetInspectorName() const noexcept;

        SR_NODISCARD uint64_t GetVersion() const noexcept;
        SR_NODISCARD SR_UTILS_NS::StringAtom GetExtension() const noexcept;
        SR_NODISCARD bool HasExtension(StringAtom extension) const noexcept;
        SR_NODISCARD bool HasExtension(StringView extension) const noexcept;
        SR_NODISCARD bool HasExtension(const String& extension) const noexcept;

        SR_NODISCARD const SR_UTILS_NS::Reflection::Method* FindMethod(SR_UTILS_NS::StringAtom name) const noexcept;

        SR_NODISCARD virtual std::span<const SR_UTILS_NS::StringAtom> GetCategory() const noexcept;
        SR_NODISCARD virtual std::span<const SR_UTILS_NS::StringAtom> GetExtensions() const noexcept;
        SR_NODISCARD virtual Reflection::TypeInfoVTable GetVTable() const noexcept;
        SR_NODISCARD virtual bool IsAbstract() const noexcept;
        SR_NODISCARD virtual bool IsHidden() const noexcept;
        SR_NODISCARD virtual bool IsEditorOnly() const noexcept;
        SR_NODISCARD virtual std::span<const SRClassMeta*> GetBaseMetas() const noexcept;
        SR_NODISCARD virtual std::span<const SR_UTILS_NS::Reflection::Property> GetProperties() const noexcept;
        SR_NODISCARD virtual std::span<const SR_UTILS_NS::Reflection::Method> GetMethods() const noexcept;
        SR_NODISCARD virtual SR_UTILS_NS::StringAtom GetFactoryName() const noexcept;
        SR_NODISCARD virtual SR_UTILS_NS::StringAtom GetDisplayName() const noexcept;
        SR_NODISCARD virtual SRClass* Allocate() const noexcept;

        SR_NODISCARD bool IsSameOrInherited(SR_UTILS_NS::StringAtom name) const;

    protected:
        SR_NODISCARD virtual uint64_t GetVersionImpl() const noexcept;

    private:
        mutable uint64_t m_versionCached = SR_UINT64_MAX;

    };
};

namespace Codegen {
    template<class T> struct SRClassMetaTemplate : public SR_UTILS_NS::SRClassMeta { };
}

#endif //SR_COMMON_TYPE_TRAITS_SR_CLASS_META_H
