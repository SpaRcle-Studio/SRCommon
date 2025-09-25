//
// Created by Nikita on 16.11.2020.
//

#ifndef HELPER_RESOURCEMANAGER_H
#define HELPER_RESOURCEMANAGER_H

#include <Utils/Debug.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Common/ToString.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Resources/IResource.h>
#include <Utils/Resources/FileSystemWatcher.h>
#include <Utils/Resources/ResourceInfo.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    class IResourceReloader;
    class FileWatcher;

    SR_INLINE_STATIC SR_UTILS_NS::StringAtom RESOURCE_ID_SEPARATOR = "→→→";

    class ResourceManager final : public Singleton<ResourceManager> {
        SR_REGISTER_SINGLETON(ResourceManager)
        using Hash = uint64_t;
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
        SR_NODISCARD FileSystemWatcher::Ptr GetFileSystemWatcher() const { return m_fileSystemWatcher; }

        SR_NODISCARD IResource::Ptr Find(SR_UTILS_NS::StringAtom id, SR_UTILS_NS::StringAtom typeName) const;
        SR_NODISCARD IResource::Ptr FindAnyType(SR_UTILS_NS::StringAtom id) const;

        void Synchronize(bool force);

        void ReloadResource(const IResource::Ptr& pResource);
        void ReloadAll(SR_UTILS_NS::StringAtom typeName);

        void Execute(const SR_HTYPES_NS::Function<void()>& fun);
        void InspectResources(const SR_HTYPES_NS::Function<void(ResourcesTypes &)>& callback);

        template<typename T> SR_HTYPES_NS::SharedPtr<T> Find(const SR_UTILS_NS::StringAtom& id) {
            SR_TRACY_ZONE;
            return Find(id, T::GetClassStaticName()).template DynamicCast<T>();
        }

        template<typename T> SR_HTYPES_NS::SharedPtr<T> Find(const Path& path) {
            SR_TRACY_ZONE;
            return Find(path.ToStringRef(), T::GetClassStaticName()).template DynamicCast<T>();
        }

        template<typename T> SR_HTYPES_NS::SharedPtr<T> GetOrLoadResource(
            const Path& rawPath,
            const SR_HTYPES_NS::Function<void(T&)>& loadCallback = SR_HTYPES_NS::Function<void(T&)>(),
            const SR_HTYPES_NS::Function<std::string()>& getPrefix = SR_HTYPES_NS::Function<std::string()>()
        );

        template<typename ResourceT, typename ReloaderT, typename ...Args> bool RegisterReloader(Args&&... args) {
            if constexpr (!std::is_base_of_v<IResource, ResourceT>) {
                static_assert(std::is_base_of_v<IResource, ResourceT>, "Resource must be derived from IResource");
            }
            return RegisterReloader(new ReloaderT(std::forward<Args>(args)...), ResourceT::GetClassStaticName());
        }

        /** \warning Call only from IResource parents \brief Register resource in resource manager */
        void RegisterResource(const IResource::Ptr& pResource);

        /** \warning Call only from IResource parents \brief Register resource to destroy in resource manager */
        bool Destroy(const IResource::Ptr& pResource);

        bool ReviveResource(const IResource::Ptr& pResource);

        bool IsSingletonCanBeDestroyed() const override { return false; }

    public:
        bool Initialize(const SR_UTILS_NS::Path& resourcesFolder, const SR_UTILS_NS::Path& engineResourcesFolder);
        void DeInitialize();

        /// Проверить хэши ресурсов и перезагрузить их, если это требуется
        void ReloadResources(float_t dt);

        void PullWatchers();

        void PrintMemoryDump();
        void EnableStackTraceProfiling();

    private:
        SR_NODISCARD ResourceType* GetOrCreateResourceType(SR_UTILS_NS::StringAtom typeName);

        bool RegisterReloader(IResourceReloader* pReloader, SR_UTILS_NS::StringAtom typeName);

        void Remove(const IResource::Ptr& pResource);
        void GC();
        void Thread();

    private:
        ResourcesList m_destroyed;
        ResourcesTypes m_resources;

        FileSystemWatcher::Ptr m_fileSystemWatcher;

        IResourceReloader* m_defaultReloader = nullptr;

        std::vector<ResourceInfo::WeakPtr> m_dirtyResources;

        std::unordered_map<Hash, SR_HTYPES_NS::Thread::Ptr> m_asyncTasks;

        std::atomic<bool> m_usePointStackTraceProfiling = false;
        std::atomic<bool> m_isInit = false;
        std::atomic<bool> m_isRun = false;
        std::atomic<bool> m_force = false;

        Path m_folder;
        Path m_engineFolder;
        Types::Thread::Ptr m_thread = nullptr;
        uint64_t m_lastTime = 0;
        uint64_t m_deltaTime = 0;

        uint64_t m_GCDt = 0;

    };

    template<typename T> SR_HTYPES_NS::SharedPtr<T> ResourceManager::GetOrLoadResource(
        const Path& rawPath,
        const SR_HTYPES_NS::Function<void(T&)>& loadCallback,
        const SR_HTYPES_NS::Function<std::string()>& getPrefix
    ) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (rawPath.IsEmpty()) {
            SR_ERROR("ResourceManager::GetOrLoadResource() : path is empty! Type: {}", T::GetClassStaticName());
            return nullptr;
        }

        Path path = Path(rawPath).RemoveSubPath(ResourceManager::Instance().GetResPath());
        Path id;

        if (auto&& prefix = getPrefix ? getPrefix() : std::string(); !prefix.empty()) {
            id = prefix + RESOURCE_ID_SEPARATOR.ToStringRef() + path.ToStringRef();
        }
        else {
            id = path;
        }

        /// Сперва попробуем найти ресурс в памяти, файл может уже не существовать, а потом уже ищем файл.
        SR_HTYPES_NS::SharedPtr<T> pResource = Find<T>(id);

        if (pResource && !pResource->IsAllowedMultiInstance()) {
            return pResource;
        }

        Path fullPath = GetResPathRef().Concat(path);
        if (!fullPath.Exists()) {
            SR_ERROR("ResourceManager::GetOrLoadResource() : resource \"{}\" not existing!\n\tPath: {}", T::GetClassStaticName(), fullPath);
            return nullptr;
        }

        pResource = T::template MakeShared<T>();
        if (loadCallback) {
            loadCallback(*pResource);
        }
        pResource->SetId(id.ToStringRef(), path, false /** auto register */);

        if (!pResource->Reload()) {
            SR_ERROR("ResourceManager::GetOrLoadResource() : failed to load {}! \n\tPath: {}",
                     T::GetClassStaticName(), path.ToStringRef());
            pResource->DeleteResource();
            return nullptr;
        }

        /// отложенная ручная регистрация
        RegisterResource(pResource.template StaticCast<IResource>());

        return pResource;
    }
}

#endif //HELPER_RESOURCEMANAGER_H
