//
// Created by Nikita on 16.11.2020.
//

#ifndef SR_ENGINE_COMMON_RESOURCE_MANAGER_H
#define SR_ENGINE_COMMON_RESOURCE_MANAGER_H

#include <Utils/Types/Thread.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Resources/IResource.h>
#include <Utils/Resources/FileSystemWatcher.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    class IResourceReloader;
    class FileWatcher;

    class ResourceManager final : public Singleton<ResourceManager> {
        SR_REGISTER_SINGLETON(ResourceManager)
        using Hash = uint64_t;
        using ResourceLoaderFn = SR_HTYPES_NS::Function<IResource::Ptr(const StringAtom&)>;
        using ResourcesTypes = SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, ResourceType*>;
    public:
        static const uint64_t ResourceLifeTime;

    public:
        SR_NODISCARD bool IsInitialized() const { return m_isInit; }
        SR_NODISCARD bool IsLastResource(const IResource::Ptr& pResource);
        SR_NODISCARD bool IsUsePointStackTraceProfilingEnabled() const { return m_usePointStackTraceProfiling; }
        SR_NODISCARD Path GetResPath() const;
        SR_NODISCARD const Path& GetResPathRef() const;
        SR_NODISCARD Path GetEngineResPath() const { return m_engineFolder; }
        SR_NODISCARD const Path& GetEngineResPathRef() const { return m_engineFolder; }
        SR_NODISCARD Path GetCachePath() const;
        SR_NODISCARD Path GetEngineCachePath() const;
        SR_NODISCARD FileSystemWatcher::Ptr GetFileSystemWatcher() const { return m_fileSystemWatcher; }
        SR_NODISCARD IResourceReloader* GetDefaultReloader() const { return m_defaultReloader.Get(); }
        SR_NODISCARD bool HasDirtyResources() const { return m_hasDirtyResources; }

        SR_NODISCARD IResource::Ptr Find(StringAtom id, StringAtom typeName, const IResourceVariant* pVariant) const;
        SR_NODISCARD IResource::Ptr FindAnyType(StringAtom id, const IResourceVariant* pVariant) const;
        SR_NODISCARD IResource::Ptr LoadResource(StringAtom id, StringAtom typeName, const IResourceVariant* pVariant = nullptr);
        SR_NODISCARD IResource::Ptr LoadResource(const Path& path, StringAtom typeName, const IResourceVariant* pVariant = nullptr);

        template<typename T> SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> Find(StringAtom id, const IResourceVariant* pVariant);
        template<typename T> SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> Find(const Path& path, const IResourceVariant* pVariant);

        void Synchronize(bool force);

        void ReloadResource(const IResource::Ptr& pResource);
        void ReloadResource(StringAtom id, StringAtom typeName);
        void ReloadAll(StringAtom typeName);
        void ChangeResourcesFolder(const Path& path);

        void Execute(const SR_HTYPES_NS::Function<void()>& fun);
        void InspectResources(const SR_HTYPES_NS::Function<void(ResourcesTypes&)>& callback);

        /** \warning Call only from IResource parents \brief Register resource in resource manager */
        void RegisterResource(const IResource::Ptr& pResource);

        /** \warning Call only from IResource parents \brief Register resource to destroy in resource manager */
        bool Destroy(const IResource::Ptr& pResource);

        bool ReviveResource(const IResource::Ptr& pResource);

        bool IsSingletonCanBeDestroyed() const override { return false; }

    public:
        bool Initialize(const Path& resourcesFolder, const Path& engineResourcesFolder);
        void DeInitialize();

        /// Проверить хэши ресурсов и перезагрузить их, если это требуется
        void ReloadResources(float_t dt);

        void PullWatchers();

        void PrintMemoryDump();
        void EnableStackTraceProfiling();

    private:
        SR_NODISCARD ResourceType* GetOrCreateResourceType(StringAtom typeName);

        void Remove(const IResource::Ptr& pResource);
        void GC();
        void Thread();

    private:
        std::vector<IResource::Ptr> m_destroyQueue;
        ResourcesTypes m_resources;

        std::map<StringAtom, ResourceLoaderFn> m_resourceLoaders;
        SR_HTYPES_NS::RawPointerHolder<IResourceReloader> m_defaultReloader;

        FileSystemWatcher::Ptr m_fileSystemWatcher;

        std::unordered_map<Hash, SR_HTYPES_NS::Thread::Ptr> m_asyncTasks;

        std::atomic<bool> m_usePointStackTraceProfiling = false;
        std::atomic<bool> m_isInit = false;
        std::atomic<bool> m_isRun = false;
        std::atomic<bool> m_force = false;
        std::atomic<bool> m_hasDirtyResources = false;

        Path m_folder;
        Path m_engineFolder;
        SR_HTYPES_NS::Thread::Ptr m_thread = nullptr;
        uint64_t m_lastTime = 0;
        uint64_t m_deltaTime = 0;

        uint64_t m_GCDt = 0;

    };

    template<typename T> SR_HTYPES_NS::SharedPtr<T> ResourceManager::Find(StringAtom id, const IResourceVariant* pVariant) {
        SR_TRACY_ZONE;
        return SR_UTILS_NS::DynamicPointerCast<T>(Find(id, T::GetClassStaticName(), pVariant));
    }

    template<typename T> SR_HTYPES_NS::SharedPtr<T> ResourceManager::Find(const Path& path, const IResourceVariant* pVariant) {
        SR_TRACY_ZONE;
        return SR_UTILS_NS::DynamicPointerCast<T>(Find(path.ToStringRef(), T::GetClassStaticName(), pVariant));
    }
}

struct CoreResLoader {
    template<typename T> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<T> Load(const char* path, const SR_UTILS_NS::IResourceVariant* pVariant = nullptr) {
        return SR_UTILS_NS::DynamicPointerCast<T>(SR_UTILS_NS::ResourceManager::Instance().LoadResource(SR_UTILS_NS::Path(path), T::GetClassStaticName(), pVariant));
    }
    template<typename T> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<T> Load(const std::string_view& path, const SR_UTILS_NS::IResourceVariant* pVariant = nullptr) {
        return SR_UTILS_NS::DynamicPointerCast<T>(SR_UTILS_NS::ResourceManager::Instance().LoadResource(SR_UTILS_NS::Path(path), T::GetClassStaticName(), pVariant));
    }
    template<typename T> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<T> Load(const std::string& path, const SR_UTILS_NS::IResourceVariant* pVariant = nullptr) {
        return SR_UTILS_NS::DynamicPointerCast<T>(SR_UTILS_NS::ResourceManager::Instance().LoadResource(SR_UTILS_NS::Path(path), T::GetClassStaticName(), pVariant));
    }
    template<typename T> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<T> Load(const SR_UTILS_NS::Path& path, const SR_UTILS_NS::IResourceVariant* pVariant = nullptr) {
        return SR_UTILS_NS::DynamicPointerCast<T>(SR_UTILS_NS::ResourceManager::Instance().LoadResource(path, T::GetClassStaticName(), pVariant));
    }
    template<typename T> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<T> Load(SR_UTILS_NS::StringAtom id, const SR_UTILS_NS::IResourceVariant* pVariant = nullptr) {
        return SR_UTILS_NS::DynamicPointerCast<T>(SR_UTILS_NS::ResourceManager::Instance().LoadResource(id, T::GetClassStaticName(), pVariant));
    }
};

#endif //SR_ENGINE_COMMON_RESOURCE_MANAGER_H
