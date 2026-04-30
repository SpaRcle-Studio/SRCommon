//
// Created by Monika on 19.02.2026.
//

#ifndef SR_ENGINE_UTILS_RESOURCE_REF_H
#define SR_ENGINE_UTILS_RESOURCE_REF_H

#include <Utils/Resources/IResource.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS {
    class ResourceRefBase : public Serializable {
        SR_CLASS()
        using Super = Serializable;
    public:
        ResourceRefBase() = default;
        ResourceRefBase(const ResourceRefBase& other);
        ResourceRefBase(ResourceRefBase&& other) noexcept;
        ResourceRefBase& operator=(const ResourceRefBase& other);
        ResourceRefBase& operator=(ResourceRefBase&& other) noexcept;
        ~ResourceRefBase() override;

        void SetResource(StringAtom id);

        void OnPostLoad() override;
        void CloneTo(SRClass& clone) const override;

        SR_NODISCARD static IResource::Ptr LoadResource(StringAtom type, StringAtom id);

        SR_NODISCARD bool IsValid() const noexcept { return m_resource.Valid(); }
        SR_NODISCARD StringAtom GetId() const noexcept { return m_id; }
        SR_NODISCARD virtual StringAtom GetResourceType() const noexcept { return {}; }
        SR_NODISCARD StringAtom GetExtension() const noexcept;
        SR_NODISCARD const IResource::Ptr& GetResourceBase() const noexcept { return m_resource; }

    protected:
        /// @property @hidden
        StringAtom m_id;
        /// @property @hidden @dontSave @dontClone
        IResource::Ptr m_resource;

    };

    template<class T> class ResourceRef : public ResourceRefBase {
        using Super = ResourceRefBase;
    public:
        ResourceRef() = default;
        ResourceRef(StringAtom id); /// NOLINT(google-explicit-constructor)
        ResourceRef(const Path& path); /// NOLINT(google-explicit-constructor)

        SR_NODISCARD StringAtom GetResourceType() const noexcept override;

        SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> GetResource() const noexcept {
            if (auto&& pBase = GetResourceBase()) {
                return SR_UTILS_NS::StaticPointerCast<T>(pBase);
            }
            return nullptr;
        }

    };

    template<class T> StringAtom ResourceRef<T>::GetResourceType() const noexcept {
        if constexpr (!std::is_same_v<T, void>) {
            if constexpr (IsCompleteTypeV<T>) {
                return T::GetClassStaticName();
            }
            else {
                return Factory::Instance().GetNameByTypeId<T>();
            }
        }
        static const StringAtom voidTypeName("void");
        return voidTypeName;
    }

    template<class T> ResourceRef<T>::ResourceRef(StringAtom id) {
        SR_TRACY_ZONE;
        m_id = id;
        m_resource = LoadResource(GetResourceType(), id);
    }

    template<class T> ResourceRef<T>::ResourceRef(const Path& path)
        : ResourceRef(path.ToStringRef())
    { }
}

#endif //SR_ENGINE_UTILS_RESOURCE_REF_H
