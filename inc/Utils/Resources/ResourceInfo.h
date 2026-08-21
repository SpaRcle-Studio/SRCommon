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

    typedef std::list<IResource::Ptr> ResourcesList;
    typedef std::unordered_set<IResource::Ptr> ResourcesSet;

    using ResourceId = SR_UTILS_NS::StringAtom;
    using ResourcePath = SR_UTILS_NS::StringAtom;

    //struct SR_COMMON_DLL_API ResourceInfo {
    //    using HardPtr = std::shared_ptr<ResourceInfo>;
    //    using WeakPtr = std::weak_ptr<ResourceInfo>;

    //    ResourceInfo(uint64_t resourceHash, SR_UTILS_NS::StringAtom path, ResourceType* pResourceType)
    //        : m_resourceType(pResourceType)
    //        //, m_resourceHash(resourceHash)
    //        , m_path(path)
    //    { }

    //    SR_NODISCARD const ResourcesSet& GetResources() const { return m_loaded; }
    //    SR_NODISCARD IResource::Ptr GetResource() const;
    //    SR_NODISCARD IResource::Ptr GetFirstResource() const;
    //    SR_NODISCARD IResourceReloader* GetReloader() const;

    //    ResourceType* m_resourceType = nullptr;

    //    /// текущий хеш самого ресурса (параметры и тд, не файл!)
    //   // uint64_t m_resourceHash = 0;
    //    /// путь ресурса
    //    SR_UTILS_NS::StringAtom m_path;

    //    ResourcesSet m_loaded;
    //};

    struct ResourcesStorage : public NonCopyable {
        SR_UTILS_NS::StringAtom id;
        SR_UTILS_NS::StringAtom path;
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
        //using CopiesMap = std::unordered_map<ResourceId, std::unordered_set<IResource::Ptr>>;
        //using Info = std::unordered_map<ResourcePath, ResourceInfo::HardPtr>;

    public:
        explicit ResourceType(StringAtom name)
            : m_name(name)
        {
            m_dirtyResources.reserve(128);
        }

    public:
        SR_NODISCARD IResource::Ptr Find(ResourceId id, const IResourceVariant* pVariant) const;
        SR_NODISCARD bool IsLast(ResourceId id, const IResourceVariant* pVariant);
        //SR_NODISCARD CopiesMap& GetCopiesRef();
        //SR_NODISCARD const CopiesMap& GetCopiesRef() const;
        //SR_NODISCARD Info& GetInfo() { return m_info; }
        //SR_NODISCARD std::pair<ResourcePath, ResourceInfo::HardPtr> GetInfoByIndex(uint64_t index);
        SR_NODISCARD IResourceReloader* GetReloader() const noexcept { return m_reloader.Get(); }
        SR_NODISCARD StringAtom GetName() const { return m_name; }
        SR_NODISCARD uint64_t GetCount() const { return m_count; }
        //SR_NODISCARD const ResourcesSet& GetResources() const { return m_resources; }

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
        //ResourcesSet m_resources;
        //CopiesMap m_copies;
        //Info m_info;
        StringAtom m_name;

    };
}

#endif //SR_ENGINE_RESOURCE_INFO_H
