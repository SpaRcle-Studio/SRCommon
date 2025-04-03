//
// Created by Nikita on 16.11.2020.
//

#ifndef HELPER_RESOURCEMANAGER_H
#define HELPER_RESOURCEMANAGER_H

#include <Utils/Debug.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Resources/IResource.h>
#include <Utils/Resources/FileSystemWatcher.h>
#include <Utils/Resources/ResourceInfo.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    class IResourceReloader;
    class FileWatcher;

    SR_INLINE_STATIC SR_UTILS_NS::StringAtom RESOURCE_ID_SEPARATOR = "→→→";

    std::optional<Path> GetResourceFolder(const Path& appFolder);

    class SR_DLL_EXPORT ResourceManager final : public Singleton<ResourceManager> {
        SR_REGISTER_SINGLETON(ResourceManager)
        using Hash = uint64_t;
    public:
        static const uint64_t ResourceLifeTime;

    public:
        SR_NODISCARD bool IsLastResource(IResource* resource);
        SR_NODISCARD bool IsUsePointStackTraceProfilingEnabled() const { return m_usePointStackTraceProfiling; }
        SR_NODISCARD Path GetResPath() const { return GetResPathRef(); }
        SR_NODISCARD const Path& GetResPathRef() const;
        SR_NODISCARD Path GetCachePath() const { return GetResPathRef().Concat("Cache"); }
        SR_NODISCARD std::string_view GetTypeName(Hash hashName) const;
        SR_NODISCARD FileSystemWatcher::Ptr GetFileSystemWatcher() const { return m_fileSystemWatcher; }

        SR_NODISCARD IResource* Find(uint64_t hashTypeName, const std::string& ID);

        void Synchronize(bool force);

        void ReloadResource(IResource* pResource);

        void Execute(const SR_HTYPES_NS::Function<void()>& fun);
        void InspectResources(const SR_HTYPES_NS::Function<void(const ResourcesTypes &)>& callback);

        template<typename T> T* Find(const std::string& id) {
            return dynamic_cast<T*>(Find(SR_COMPILE_TIME_CRC32_TYPE_NAME(T), id));
        }

        template<typename T> T* Find(const Path& path) {
            SR_TRACY_ZONE;
            return dynamic_cast<T*>(Find(SR_COMPILE_TIME_CRC32_TYPE_NAME(T), path.ToStringRef()));
        }

        template<typename T> bool RegisterType() {
            if constexpr (!std::is_base_of_v<IResource, T>) {
                static_assert(std::is_base_of_v<IResource, T>, "Resource must be derived from IResource");
            }
            return RegisterType(typeid(T).name(), SR_COMPILE_TIME_CRC32_TYPE_NAME(T));
        }

        template<typename ResourceT, typename ReloaderT, typename ...Args> bool RegisterReloader(Args&&... args) {
            if constexpr (!std::is_base_of_v<IResource, ResourceT>) {
                static_assert(std::is_base_of_v<IResource, ResourceT>, "Resource must be derived from IResource");
            }
            return RegisterReloader(new ReloaderT(std::forward<Args>(args)...), SR_COMPILE_TIME_CRC32_TYPE_NAME(ResourceT));
        }

        /** \warning Call only from IResource parents \brief Register resource in resource manager */
        void RegisterResource(IResource* pResource);

        /** \warning Call only from IResource parents \brief Register resource to destroy in resource manager */
        bool Destroy(IResource* pResource);

        bool ReviveResource(IResource* pResource);

    public:
        bool Init(const SR_UTILS_NS::Path& resourcesFolder);
        bool Run();

        void OnSingletonDestroy() override;

        /// Проверить хэши ресурсов и перезагрузить их, если это требуется
        void ReloadResources(float_t dt);

        void PullWatchers();

        void PrintMemoryDump();
        void EnableStackTraceProfiling();

    private:
        bool RegisterType(const std::string& name, uint64_t hashTypeName);
        bool RegisterReloader(IResourceReloader* pReloader, uint64_t hashTypeName);

        void Remove(IResource *resource);
        void GC();
        void Thread();

    private:
        ResourcesList m_destroyed;
        ResourcesTypes m_resources;

        FileSystemWatcher::Ptr m_fileSystemWatcher;

        IResourceReloader* m_defaultReloader = nullptr;

        std::queue<ResourceInfo::WeakPtr> m_dirtyResources;

        std::unordered_map<Hash, SR_HTYPES_NS::Thread::Ptr> m_asyncTasks;

        std::atomic<bool> m_usePointStackTraceProfiling = false;
        std::atomic<bool> m_isInit = false;
        std::atomic<bool> m_isRun = false;
        std::atomic<bool> m_force = false;

        Path m_folder;
        Types::Thread::Ptr m_thread = nullptr;
        uint64_t m_lastTime = 0;
        uint64_t m_deltaTime = 0;

        uint64_t m_GCDt = 0;

    };
}

#endif //HELPER_RESOURCEMANAGER_H
