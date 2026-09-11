//
// Created by Monika on 02.05.2022.
//

#ifndef SR_ENGINE_RESOURCE_INFO_H
#define SR_ENGINE_RESOURCE_INFO_H

#include <Utils/Resources/IResource.h>
#include <Utils/Types/SetVector.h>
#include <Utils/Types/RawPointerHolder.h>

namespace SR_UTILS_NS {
    class ResourceManager;
    class IResourceReloader;
    class ResourceType;

    struct ResourcesStorage : public NonCopyable {
        StringAtom id;
        StringAtom path;
        Map<uint64_t, Vector<IResource::Ptr>> variants;

        SR_NODISCARD bool IsLast() const;
        SR_NODISCARD IResource::Ptr Find(const IResourceVariant* pVariant) const;

        void ForEach(const SR_HTYPES_NS::Function<void(IResource&)>& fun);
        void CollectUnused();

        void Add(const IResource::Ptr& pResource);
        void Remove(const IResource::Ptr& pResource);
    };

    class SR_COMMON_DLL_API ResourceType : public NonCopyable {
        friend class ResourceManager;
    public:
    public:
        explicit ResourceType(StringAtom name)
            : m_name(name)
        {
            m_dirtyResources.reserve(128);
        }

    public:
        SR_NODISCARD IResource::Ptr Find(StringAtom id, const IResourceVariant* pVariant) const;
        SR_NODISCARD bool IsLast(StringAtom id, const IResourceVariant* pVariant);
        SR_NODISCARD StringAtom GetName() const { return m_name; }
        SR_NODISCARD uint64_t GetCount() const { return m_count; }

        void Remove(const IResource::Ptr& pResource);
        void Add(const IResource::Ptr& pResource);

        void ForEach(const SR_HTYPES_NS::Function<void(const IResource&)>& fun) const;
        void ForEach(const SR_HTYPES_NS::Function<void(IResource&)>& fun);

        void ReloadAll();
        void Reload(const IResource::Ptr& pResource);
        void Reload(StringAtom resourceId);

        void ReloadDirtyResources();

        void SetReloader(IResourceReloader* pReloader);

        /// ставит все неиспользуемые ресурсы на очередь уничтожения
        void CollectUnused();

    private:
        uint64_t m_count = 0;
        SR_HTYPES_NS::SetVector<StringAtom> m_dirtyResources;
        RawPointerHolder<IResourceReloader> m_reloader;
        SR_HTYPES_NS::FlatHashMap<StringAtom, ResourcesStorage*> m_storage;
        StringAtom m_name;

    };
}

#endif //SR_ENGINE_RESOURCE_INFO_H
