//
// Created by Nikita on 16.11.2020.
//

#include <Utils/Resources/ResourceManager.h>

#include <Utils/Resources/IResourceReloader.h>
#include <Utils/Resources/FileWatcher.h>
#include <Utils/Common/Features.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/StringUtils.h>

namespace SR_UTILS_NS {
    std::optional<Path> GetResourceFolder(const Path& appFolder) {
        for (int32_t i = 0; i < 5; i++) {
            auto&& pathToConcat = StringUtils::MultiConcat("../", i) + "Resources";

            auto&& resPath = appFolder.Concat(pathToConcat);
            if (resPath.Exists(SR_UTILS_NS::Path::Type::Folder)) {
                return resPath;
            }
        }

        return std::nullopt;
    }


    /// Seconds
    const uint64_t ResourceManager::ResourceLifeTime = 30 * SR_CLOCKS_PER_SEC;

    bool ResourceManager::Init(const SR_UTILS_NS::Path& resourcesFolder) {
    #ifdef SR_ANDROID
        SR_INFO("ResourceManager::Init() : initializing resource manager...");
    #else
        SR_INFO("ResourceManager::Init() : initializing resource manager...\n\tResources folder: " + resourcesFolder.ToString());
    #endif

        m_defaultReloader = new DefaultResourceReloader();

        m_folder = resourcesFolder;

        m_resources.max_load_factor(0.9f);

        m_isInit = true;

        return true;
    }

    SR_HTYPES_NS::SharedPtr<FileWatcher> ResourceManager::StartWatch(const Path& path) {
        SR_SCOPED_LOCK;
        SRAssert(m_isRun);

        if (!path.IsFile()) {
            SRHalt("ResourceManager::StartWatch() : watching a non-existent file! '{}'", path.ToStringRef());
            return nullptr;
        }

        if (path == GetResPath()) {
            SRHalt("ResourceManager::StartWatch() : watching the resource folder is prohibited!");
            return nullptr;
        }

        FileWatcher::Ptr pWatcher = new FileWatcher(path);
        m_watchers.emplace_back(pWatcher);
        return pWatcher;
    }

    void ResourceManager::AsyncUpdateWatchers() {
        SR_SCOPED_LOCK;
        SR_TRACY_ZONE;

        if (m_watchers.empty() || !IsWatchingEnabled()) {
            return;
        }

        FileWatcher::Ptr pWatcher = m_watchers.front();
        SRAssert(pWatcher);

        m_watchers.erase(m_watchers.begin());

        if (!pWatcher) {
            return;
        }

        /// Watcher может быть уничтожен в конце этой функции
        /// Так же, учитываем что его состояние может быть изменено сразу после IsActive
        {
            std::lock_guard lockWatcher(pWatcher->GetMutex());

            if (!pWatcher->IsActive()) {
                return;
            }

            if (!pWatcher->IsDirty() && !pWatcher->IsPaused() && pWatcher->Update()) {
                m_dirtyWatchers.push(pWatcher);
            }
        }

        m_watchers.emplace_back(pWatcher);
    }

    void ResourceManager::OnSingletonDestroy() {
        SR_INFO("ResourceManager::OnSingletonDestroy() : stopping resource manager...");

        PrintMemoryDump();

        m_isInit = false;
        m_isRun = false;

        Synchronize(true);

        SR_INFO("ResourceManager::OnSingletonDestroy() : stopping thread...");

        if (m_thread) {
            m_thread->TryJoin();
            m_thread->Free();
            m_thread = nullptr;
        }

        PrintMemoryDump();

        SR_SAFE_DELETE_PTR(m_defaultReloader);

        for (auto&& [hashTypeName, pResourceType] : m_resources) {
            delete pResourceType;
        }
        m_resources.clear();

        for (auto&& pFileWatcher : m_watchers) {
            if (!pFileWatcher->IsActive()) {
                continue;
            }

            SR_ERROR("ResourceManager::OnSingletonDestroy() : file watcher was not stopped!"
                 "\n\tPath: " + pFileWatcher->GetPath().ToStringRef()
                + "\n\tName: " + pFileWatcher->GetName()
            );
        }
        m_watchers.clear();
    }

    bool ResourceManager::Destroy(IResource* pResource) {
        SR_TRACY_ZONE;

        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("ResourceManager::Destroy() : destroying \"" + std::string(pResource->GetResourceName()) + "\"");
        }

        SR_SCOPED_LOCK;

        for (auto&& pDestroyedResource : m_destroyed) {
            if (pResource == pDestroyedResource) {
                SRHalt("ResourceManager::Destroy() : resource is already destroyed!");
                return false;
            }
        }

        m_destroyed.emplace_back(pResource);

        return true;
    }

    bool ResourceManager::RegisterType(const std::string& name, uint64_t hashTypeName) {
        SR_INFO("ResourceManager::RegisterType() : registering new \"" + name + "\" type...");

        if (m_resources.count(hashTypeName) == 1) {
            SRHalt("ResourceManager::RegisterType() : type is already registered!");
            return false;
        }

        m_resources.insert(std::make_pair(
            hashTypeName,
            new ResourceType(name)
        ));

        return true;
    }

    void ResourceManager::Remove(IResource* pResource) {
        SR_TRACY_ZONE;

        if (pResource->IsRegistered()) {
            auto&& pGroupIt = m_resources.find(pResource->GetResourceHashName());
            auto&& [name, resourcesGroup] = *pGroupIt;
            resourcesGroup->Remove(pResource);
        }
        else {
           SRHalt("Resource ins't registered! "
                "\n\tType: " + std::string(pResource->GetResourceName()) +
                "\n\tId: " + std::string(pResource->GetResourceId()));
        }
    }

    bool ResourceManager::IsLastResource(IResource* pResource) {
        auto&& [name, resourcesGroup] = *m_resources.find(pResource->GetResourceHashName());
        return resourcesGroup->IsLast(pResource->GetResourceId());
    }

    const Path& ResourceManager::GetResPathRef() const {
        SRAssert2(m_isInit, "Resource manager isn't initialized : " + m_folder.ToString());

        return m_folder;
    }

    void ResourceManager::Thread() {
        do {
            SR_PLATFORM_NS::Sleep(5);

            SR_TRACY_ZONE;

            auto time = clock();
            m_deltaTime = static_cast<uint64_t>(time - m_lastTime); /// miliseconds
            m_lastTime = time;

            m_GCDt += m_deltaTime;
            m_hashCheckDt += m_deltaTime;

            if (m_hashCheckDt > 15 /** ms */) {
                AsyncUpdateWatchers();
                m_hashCheckDt = 0;
            }

            if (m_GCDt > (m_force ? 100 : 500) /** ms */) {
                /** если какой-то ресурс больше не используется, то уничтожаем его.
                 * все происходящее в GC должно быть потоко-безопасным, то есть при освобождении
                 * ресурсов не должны блокироваться другие потоки, иначе будут проблемы. */
                GC();
                m_GCDt = 0;
            }
        }
        while(m_isRun);

        SR_INFO("ResourceManager::Thread() : exit from thread-function.");
    }

    void ResourceManager::GC() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        /// Не можем работать, пока какие-то ресурсы не перезагружены
        if (!m_dirtyResources.empty() || !m_dirtyWatchers.empty()) {
            return;
        }

        if (m_destroyed.empty()) {
            return;
        }

        if (m_force) {
            for (auto&& [hashName, group] : m_resources) {
                group->CollectUnused();
            }
        }

        auto resourceIt = m_destroyed.begin();
        for (; resourceIt != m_destroyed.end(); ) {
            auto pResource = *resourceIt;

            /// ресурс был оживлен
            if (!pResource->IsDestroyed()) {
                m_destroyed.erase(resourceIt);
                resourceIt = m_destroyed.begin();
                continue;
            }

            const bool usageNow = pResource->GetCountUses() > 0 || !pResource->IsDestroyed();

            if (usageNow) {
                pResource->SetLifetime(ResourceLifeTime);
            }
            else if (IsLastResource(pResource)) {
                pResource->SetLifetime(pResource->GetLifetime() - m_GCDt);
            }
            else {
                /// нам не нужно ждать завершения времени жизни ресурса, у которого еще есть копии
                pResource->SetLifetime(0);
            }

            const bool resourceAlive = !pResource->IsForceDestroyed() && pResource->IsAlive() && !m_force;

            if (usageNow || resourceAlive) {
                ++resourceIt;
                continue;
            }

            if (Debug::Instance().GetLevel() >= Debug::Level::Medium) {
                SR_LOG("ResourceManager::GC() : free \"" + std::string(pResource->GetResourceId()) + "\" resource");
            }

            Remove(pResource);

            {
                /// так как некоторые ресурсы рекурсивно уничтожают дочерныие ресурсы при вызове деструктора, например материал,
                /// то он добавит в m_resourcesToDestroy новый элемент (в этом же потоке), соответственно любой итератор
                /// инвалидируется, и здесь может потенциально случиться краш, поэтому этот порядок нужно строго союлюдать

                m_destroyed.erase(resourceIt);
                pResource->DeleteResource();
                resourceIt = m_destroyed.begin();
            }
        }

        if (Debug::Instance().GetLevel() >= Debug::Level::High && m_destroyed.empty()) {
            SR_LOG("ResourceManager::GC() : complete garbage collection.");
        }
    }

    void ResourceManager::RegisterResource(IResource *pResource) {
        SRAssert(!pResource->IsRegistered());

        if (Debug::Instance().GetLevel() >= Debug::Level::Full) {
            SR_LOG("ResourceManager::RegisterResource() : add new \"" + std::string(pResource->GetResourceName()) + "\" resource.");
        }

        SR_SCOPED_LOCK;

    #ifdef SR_DEBUG
        if (m_resources.count(pResource->GetResourceHashName()) == 0) {
            SRAssert2(false, "Unknown resource type!");
            return;
        }
    #endif

        pResource->StartWatch();

        auto&& pGroupIt = m_resources.find(pResource->GetResourceHashName());
        auto&& [name, resourcesGroup] = *pGroupIt;

        resourcesGroup->Add(pResource);
    }

    void ResourceManager::PrintMemoryDump() {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        uint64_t count = 0;

        std::string dump = "\n================================ MEMORY DUMP ================================";

        for (const auto& [hashName, type] : m_resources) {
            dump += "\n\t\"" + std::string(type->GetName()) + "\": " + std::to_string(type->GetCopiesRef().size());

            uint32_t id = 0;
            for (auto& pRes : type->m_resources) {
                dump += SR_UTILS_NS::Format("\n\t\t{}: {} = {}", id++, pRes->GetResourceId().data(), pRes->GetCountUses());
                ++count;
            }
        }

        std::string wait;
        for (auto&& pResource : m_destroyed) {
            wait += "\n\t\t" + pResource->GetResourceId().ToStringRef() + "; uses = " +std::to_string(pResource->GetCountUses());
            ++count;
        }

        dump += "\n\tWait destroy: " + std::to_string(m_destroyed.size()) + wait;

        dump += "\n=============================================================================";

        if (count > 0) {
            SR_SYSTEM_LOG(dump);
        }
        else {
            SR_SYSTEM_LOG("ResourceManager::PrintMemoryDump() : memory dump is empty!");
        }
    }

    IResource *ResourceManager::Find(uint64_t hashTypeName, const std::string& id) {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

    #if defined(SR_DEBUG)
        if (m_resources.count(hashTypeName) == 0) {
            SRHalt("Unknown resource type!");
            return nullptr;
        }
    #endif

        auto&& [name, resourcesGroup] = *m_resources.find(hashTypeName);

        if (auto&& pResource = resourcesGroup->Find(id)) {
            /// раз ресурс ищем, значит он все еще может быть нужен.
            pResource->UpdateResourceLifeTime();
            return pResource;
        }

        return nullptr;
    }

    void ResourceManager::Synchronize(bool force) {
        SR_TRACY_ZONE;

        {
            SR_SCOPED_LOCK;
            m_force = true;
        }

        /// TODO: добавить таймер, по истечению которого поток будет умирать, чтобы не стоять в deadlock'е

        for (uint8_t i = 0; i < 255; ++i) 
        {
            for (;;)
            {
                {
                    SR_SCOPED_LOCK;
                    if (m_destroyed.empty()) {
                        break;
                    }
                }

                if (!m_thread->Joinable()) {
                    SR_ERROR("ResourceManager::Synchronize() : thread is dead!");
                    break;
                }
            }
        }

        {
            SR_LOCK_GUARD;
            m_force = false;
        }
    }

    void ResourceManager::Execute(const SR_HTYPES_NS::Function<void()>& fun) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        fun();
    }

    void ResourceManager::InspectResources(const SR_HTYPES_NS::Function<void(const ResourcesTypes &)> &callback) {
        SR_LOCK_GUARD;

        callback(m_resources);
    }

    std::string_view ResourceManager::GetTypeName(uint64_t hashName) const {
        SR_LOCK_GUARD;

        if (auto&& pIt = m_resources.find(hashName); pIt != m_resources.end()) {
            return pIt->second->GetName();
        }

        SRHalt("ResourceManager::GetTypeName() : unknown hash name!");

        return "Unknown";
    }

    bool ResourceManager::Run() {
        if (m_isRun) {
            SRHalt("ResourceManager::Run() : is already ran!");
            return false;
        }

        m_isRun = true;

        m_thread = SR_HTYPES_NS::Thread::Factory::Instance().Create(std::thread(&ResourceManager::Thread, this));
        m_thread->SetName("Resources manager");

        return true;
    }

    bool ResourceManager::RegisterReloader(IResourceReloader *pReloader, uint64_t hashTypeName) {
        SR_LOCK_GUARD;

        if (auto&& pIt = m_resources.find(hashTypeName); pIt != m_resources.end()) {
            auto&& [_, resourceType] = *pIt;
            resourceType->SetReloader(pReloader);
            return true;
        }

        SRHalt("ResourceManager::RegisterReloader() : unknown hash name!");

        return false;
    }

    void ResourceManager::ReloadResources(float_t dt) {
        SR_TRACY_ZONE;

        /// не блокируем поток, иначе не будет смысла от разделения.
        /// если прочитаем некорректные данные из empty, будем считать, что не повезло.
        if (m_dirtyResources.empty()) {
            return;
        }

        SR_LOCK_GUARD;

        while (!m_dirtyResources.empty()) {
            ResourceInfo::WeakPtr pResourceInfo = m_dirtyResources.front();
            m_dirtyResources.pop();

            /// ресурс мог быть освобожден в GC
            auto&& pHardPtr = pResourceInfo.lock();
            if (!pHardPtr) {
                continue;
            }

            IResourceReloader* pResourceReloader = nullptr;

            if (auto&& pGroupReloader = pHardPtr->GetReloader()) {
                pResourceReloader = pGroupReloader;
            }
            else {
                pResourceReloader = m_defaultReloader;
            }

            if (pHardPtr->m_path.empty()) {
                SR_ERROR("ResourceManager::ReloadResources() : resource have empty path!\n\tResource name: " +
                    pHardPtr->m_resourceType->GetName() + "\n\tHash state: " + std::to_string(pHardPtr->m_resourceHash)
                );
                continue;
            }

            if (pResourceReloader && !pResourceReloader->Reload(pHardPtr->m_path, pHardPtr.get())) {
                SR_ERROR("ResourceManager::ReloadResources() : failed to reload resource!\n\tPath: " + pHardPtr->m_path.ToStringRef());
            }
        }
    }

    void ResourceManager::UpdateWatchers(float_t dt) {
        SR_TRACY_ZONE;

        /// не блокируем поток, иначе не будет смысла от разделения.
        /// если прочитаем некорректные данные из empty, будем считать, что не повезло.
        if (m_dirtyWatchers.empty()) {
            return;
        }

        SR_LOCK_GUARD;

        while (!m_dirtyWatchers.empty()) {
            if (const FileWatcher::Ptr pWatcher = m_dirtyWatchers.front()) {
                std::lock_guard lockWatcher(pWatcher->GetMutex());

                if (!pWatcher->IsActive()) {
                    goto skip;
                }

                if (pWatcher->IsPaused()) {
                    goto skip;
                }

                pWatcher->Signal();
            }

        skip:
            m_dirtyWatchers.pop();
        }
    }

    void ResourceManager::ReloadResource(IResource* pResource) {
        SR_LOCK_GUARD;
        m_dirtyResources.push(pResource->GetResourceInfo());
    }

    void ResourceManager::EnableStackTraceProfiling() {
        if (m_usePointStackTraceProfiling) {
            return;
        }

        SR_WARN("ResourceManager::EnableStackTraceProfiling() : profiling was enabled! ONLY FOR DEV!");

        m_usePointStackTraceProfiling = true;
    }

    bool ResourceManager::IsWatchingEnabled() const {
        if (!SR_UTILS_NS::Features::Instance().Enabled("FileWatching", true)) {
            return false;
        }

        return m_isWatchingEnabled;
    }

    bool ResourceManager::ReviveResource(IResource* pResource) {
        SR_LOCK_GUARD;

        return pResource->Execute([pResource, this](){
            if (!pResource->IsDestroyed()) {
                return true;
            }

            if (pResource->IsAllowedToRevive()) {
                auto&& pIt = std::find(m_destroyed.begin(), m_destroyed.end(), pResource);
                if (pIt == m_destroyed.end()) {
                    SRHalt("ResourceManager::ReviveResource() : resource not found!");
                    return false;
                }

                m_destroyed.erase(pIt);

                if (SR_UTILS_NS::Debug::Instance().GetLevel() >= SR_UTILS_NS::Debug::Level::Medium) {
                    SR_LOG("ResourceManager::ReviveResource() : revive resource \"" + pResource->GetResourceId().ToStringRef() + "\"");
                }

                pResource->ReviveResource();

                return true;
            }

            return false;
        });
    }
}