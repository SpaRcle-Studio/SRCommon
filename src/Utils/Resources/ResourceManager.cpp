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
    /// Seconds
    const uint64_t ResourceManager::ResourceLifeTime = 30 * SR_CLOCKS_PER_SEC;

    bool ResourceManager::Initialize(const SR_UTILS_NS::Path& resourcesFolder, const SR_UTILS_NS::Path& engineResourceFolder) {
        SR_INFO("ResourceManager::Initialize() : initializing resource manager..."
            "\n\tResources folder: {}\n\tEngine resources folder: {}",
            resourcesFolder, engineResourceFolder
        );

        if (m_isInit) {
            SRHalt("ResourceManager::Initialize() : is already initialized!");
            return false;
        }

        m_defaultReloader = new DefaultResourceReloader();

        m_engineFolder = engineResourceFolder;
        m_folder = resourcesFolder;

        m_fileSystemWatcher = FileSystemWatcher::MakeShared();
        m_fileSystemWatcher->AddListener(m_folder);
        m_fileSystemWatcher->StartAsyncWatch();

        m_resources.max_load_factor(0.9f);

        m_isInit = true;

        if (m_isRun) {
            SRHalt("ResourceManager::Initialize() : is already ran!");
            return false;
        }

        m_isRun = true;

        if (!SR_HTYPES_NS::Thread::Factory::Instance().Create(m_thread, &ResourceManager::Thread, this)) {
            SRHalt("ResourceManager::Initialize() : failed to create thread!");
            return false;
        }

        m_thread->SetName("Resources manager");

        return true;
    }

    void ResourceManager::DeInitialize() {
        if (!m_isInit) {
            SRHalt("ResourceManager::DeInitialize() : is not initialized!");
            return;
        }

        if (!m_isRun) {
            SRHalt("ResourceManager::DeInitialize() : is not running!");
            return;
        }

        SR_INFO("ResourceManager::DeInitialize() : stopping resource manager...");

        PrintMemoryDump();

        Synchronize(true);

        m_isInit = false;
        m_isRun = false;

        SR_INFO("ResourceManager::DeInitialize() : stopping thread...");

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

        m_fileSystemWatcher.AutoFree();
    }

    bool ResourceManager::Destroy(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;

        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("ResourceManager::Destroy() : destroying \"{}\"", pResource->GetResourceType());
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

    void ResourceManager::Remove(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;

        if (pResource->IsRegistered()) {
            auto&& pGroupIt = m_resources.find(pResource->GetResourceType());
            auto&& [name, resourcesGroup] = *pGroupIt;
            resourcesGroup->Remove(pResource);
        }
        else {
            SRHalt("ResourceManager::Remove() : resource isn't registered!\n\tType: {}, Id: {}",
                   pResource->GetResourceType(),
                   pResource->GetResourceId());
        }
    }

    bool ResourceManager::IsLastResource(const IResource::Ptr& pResource) {
        if (auto&& pIt = m_resources.find(pResource->GetResourceType()); pIt != m_resources.end()) {
            return pIt->second->IsLast(pResource->GetResourceId());
        }
        SRHalt("ResourceManager::IsLastResource() : resource type not found!\n\tType: {}, Id: {}",
               pResource->GetResourceType(),
               pResource->GetResourceId());
        return false;
    }

    const Path& ResourceManager::GetResPathRef() const {
        SRAssert2(m_isInit, "Resource manager isn't initialized : " + m_folder.ToString());

        return m_folder;
    }

    void ResourceManager::Thread() {
        do {
            SR_TRACY_ZONE;

            m_thread->Synchronize();

            SR_PLATFORM_NS::Sleep(5);

            auto time = clock();
            m_deltaTime = static_cast<uint64_t>(time - m_lastTime); /// miliseconds
            m_lastTime = time;

            m_GCDt += m_deltaTime;

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
        if (!m_dirtyResources.empty()) {
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

    void ResourceManager::RegisterResource(const IResource::Ptr& pResource) {
        SRAssert(!pResource->IsRegistered());

        if (Debug::Instance().GetLevel() >= Debug::Level::Full) {
            SR_LOG("ResourceManager::RegisterResource() : add new \"{}\" resource.", pResource->GetResourceType());
        }

        SR_SCOPED_LOCK;

        pResource->StartWatch();
        GetOrCreateResourceType(pResource->GetResourceType())->Add(pResource);
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

    IResource::Ptr ResourceManager::Find(SR_UTILS_NS::StringAtom id, SR_UTILS_NS::StringAtom typeName) const {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        auto&& pIt = m_resources.find(typeName);
        if (pIt == m_resources.end()) {
            return nullptr;
        }

        auto&& [name, resourcesGroup] = *pIt;

        if (auto&& pResource = resourcesGroup->Find(id)) {
            /// раз ресурс ищем, значит он все еще может быть нужен.
            pResource->UpdateResourceLifeTime();
            return pResource;
        }

        return nullptr;
    }

    IResource::Ptr ResourceManager::FindAnyType(SR_UTILS_NS::StringAtom id) const {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        for (auto&& [typeName, resourcesGroup] : m_resources) {
            if (auto&& pResource = resourcesGroup->Find(id)) {
                /// раз ресурс ищем, значит он все еще может быть нужен.
                pResource->UpdateResourceLifeTime();
                return pResource;
            }
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

    void ResourceManager::InspectResources(const SR_HTYPES_NS::Function<void(ResourcesTypes &)> &callback) {
        SR_LOCK_GUARD;
        callback(m_resources);
    }

    bool ResourceManager::RegisterReloader(IResourceReloader* pReloader, SR_UTILS_NS::StringAtom typeName) {
        SR_LOCK_GUARD;
        GetOrCreateResourceType(typeName)->SetReloader(pReloader);
        return true;
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
            ResourceInfo::WeakPtr pResourceInfo = m_dirtyResources.back();
            m_dirtyResources.pop_back();

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

    void ResourceManager::PullWatchers() {
        SR_TRACY_ZONE;
        m_fileSystemWatcher->WatchPull();
    }

    void ResourceManager::ReloadAll(SR_UTILS_NS::StringAtom typeName) {
        SR_LOCK_GUARD;
        SR_TRACY_ZONE;

        auto&& pIt = m_resources.find(typeName);
        if (pIt == m_resources.end()) {
            return;
        }

        for (auto&& pResource : pIt->second->GetResources()) {
            ReloadResource(pResource);
        }
    }

    void ResourceManager::ReloadResource(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        for (auto&& pDirtyResource : m_dirtyResources) {
            if (pDirtyResource.lock() == pResource->GetResourceInfo().lock()) {
                return;
            }
        }
        m_dirtyResources.emplace_back(pResource->GetResourceInfo());
    }

    void ResourceManager::EnableStackTraceProfiling() {
        if (m_usePointStackTraceProfiling) {
            return;
        }

        SR_WARN("ResourceManager::EnableStackTraceProfiling() : profiling was enabled! ONLY FOR DEV!");

        m_usePointStackTraceProfiling = true;
    }

    bool ResourceManager::ReviveResource(const IResource::Ptr& pResource) {
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

    Path ResourceManager::GetResPath() const {
        return GetResPathRef();
    }

    Path ResourceManager::GetCachePath() const {
        if (auto&& cache = SR_PLATFORM_NS::GetApplicationCachePath()) {
            return cache->Concat("Cache");
        }
        return GetResPathRef().Concat("Cache");
    }

    ResourceType* ResourceManager::GetOrCreateResourceType(SR_UTILS_NS::StringAtom typeName) {
        SR_LOCK_GUARD;

        auto&& pIt = m_resources.find(typeName);
        if (pIt != m_resources.end()) {
            return pIt->second;
        }

        auto&& pResourceType = new ResourceType(typeName);
        m_resources.emplace(std::make_pair(typeName, pResourceType));

        SR_INFO("ResourceManager::GetOrCreateResourceType() : registered new resource type \"{}\"", typeName);

        return pResourceType;
    }
}