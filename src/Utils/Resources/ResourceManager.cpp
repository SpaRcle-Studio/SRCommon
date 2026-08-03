//
// Created by Nikita on 16.11.2020.
//

#include <Utils/Resources/ResourceManager.h>

#include <Utils/Resources/IResourceReloader.h>
#include <Utils/Resources/ResourceInfo.h>
#include <Utils/Resources/FileWatcher.h>
#include <Utils/Resources/Asset.h>
#include <Utils/Resources/Asset.h>
#include <Utils/Common/Features.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS {
    /// Seconds
    const uint64_t ResourceManager::ResourceLifeTime = 30 * SR_CLOCKS_PER_SEC;

    bool ResourceManager::Initialize(const SR_UTILS_NS::Path& resourcesFolder, const SR_UTILS_NS::Path& engineResourceFolder) {
        SR_TRACY_ZONE;

        SR_INFO("ResourceManager::Initialize() : initializing resource manager..."
            "\n\tResources folder: {}\n\tEngine resources folder: {}",
            resourcesFolder, engineResourceFolder
        );

        if (m_isInit) {
            SRHalt("ResourceManager::Initialize() : is already initialized!");
            return false;
        }

        m_resourceLoaders[Asset::GetClassStaticName()] = [](const StringAtom& id) -> IResource::Ptr {
            return StaticPointerCast<IResource>(Asset::Load(Path(id)));
        };

        m_destroyQueue.reserve(256);
        m_defaultReloader = new DefaultResourceReloader();

        m_engineFolder = engineResourceFolder;

        ChangeResourcesFolder(resourcesFolder);

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

        for (auto&& pDestroyedResource : m_destroyQueue) {
            if (pResource == pDestroyedResource) {
                SRHalt("ResourceManager::Destroy() : resource is already destroyed!");
                return false;
            }
        }

        m_destroyQueue.emplace_back(pResource);

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
            return pIt->second->IsLast(pResource->GetResourceId(), pResource->GetVariant());
        }
        SRHalt("ResourceManager::IsLastResource() : resource type not found!\n\tType: {}\n\tId: {}",
               pResource->GetResourceType(),
               pResource->GetResourceId());
        return false;
    }

    const Path& ResourceManager::GetResPathRef() const {
        SRAssert2(m_isInit, "Resource manager isn't initialized : {}", m_folder);
        return m_folder;
    }

    bool ResourceManager::Thread() {
        SR_TRACY_ZONE;

        m_thread->Synchronize();

    #ifdef SR_THREADS_ALLOWED
        SR_PLATFORM_NS::Sleep(5);
    #endif

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

        if (m_isRun) {
            return true;
        }
        SR_LOG("ResourceManager::Thread() : thread is stopped.");
        return false;
    }

    void ResourceManager::GC() {
        SR_TRACY_ZONE;

        /// Не можем работать, пока какие-то ресурсы не перезагружены
        if (m_hasDirtyResources) {
            return;
        }

        SR_LOCK_GUARD;

        if (m_destroyQueue.empty()) {
            return;
        }

        if (m_force) {
            for (auto&& [hashName, group] : m_resources) {
                group->CollectUnused();
            }
        }

        auto resourceIt = m_destroyQueue.begin();
        for (; resourceIt != m_destroyQueue.end(); ) {
            auto pResource = *resourceIt;

            /// ресурс был оживлен
            if (!pResource->IsDestroyed()) {
                m_destroyQueue.erase(resourceIt);
                resourceIt = m_destroyQueue.begin();
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

                m_destroyQueue.erase(resourceIt);
                pResource->DeleteResource();
                resourceIt = m_destroyQueue.begin();
            }
        }

        if (Debug::Instance().GetLevel() >= Debug::Level::High && m_destroyQueue.empty()) {
            SR_LOG("ResourceManager::GC() : complete garbage collection.");
        }
    }

    void ResourceManager::RegisterResource(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;

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

        for (const auto& [hashName, pResourceType] : m_resources) {
            dump += "\n\t\"{}\": {}"_format(pResourceType->GetName(), pResourceType->GetCount());

            uint32_t id = 0;
            pResourceType->ForEach([&](const IResource& resource) {
                dump += SR_UTILS_NS::Format("\n\t\t{}: {} = {}", id++, resource.GetResourceId().data(), resource.GetCountUses());
                ++count;
            });
        }

        std::string wait;
        for (auto&& pResource : m_destroyQueue) {
            wait += "\n\t\t" + pResource->GetResourceId().ToStringRef() + "; uses = " +std::to_string(pResource->GetCountUses());
            ++count;
        }

        dump += "\n\tWait destroy: " + std::to_string(m_destroyQueue.size()) + wait;

        dump += "\n=============================================================================";

        if (count > 0) {
            SR_SYSTEM_LOG(dump);
        }
        else {
            SR_SYSTEM_LOG("ResourceManager::PrintMemoryDump() : memory dump is empty!");
        }
    }

    IResource::Ptr ResourceManager::LoadResource(const Path& rawPath, SR_UTILS_NS::StringAtom typeName, const IResourceVariant* pVariant) {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;
        const SR_UTILS_NS::Path path = rawPath.RemoveSubPath(GetResPathRef());
        return LoadResource(StringAtom(path.View()), typeName, pVariant);
    }

    IResource::Ptr ResourceManager::LoadResource(SR_UTILS_NS::StringAtom id, SR_UTILS_NS::StringAtom typeName, const IResourceVariant* pVariant) {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        if (typeName.empty()) {
            SRHalt("ResourceManager::LoadResource() : resource type is empty!");
            return nullptr;
        }

        if (id.empty()) {
            SRHalt("ResourceManager::LoadResource() : resource id is empty!");
            return nullptr;
        }

        if (auto&& pFound = Find(id, typeName, pVariant)) {
            return pFound;
        }

        SR_DEBUG("ResourceManager::LoadResource() : loading \"{}\" resource with id \"{}\"", typeName, id);

        if (auto&& pIt = m_resourceLoaders.find(typeName); pIt != m_resourceLoaders.end()) {
            return pIt->second(id);
        }

        auto&& pResource = Factory::Instance().Create<IResource>(typeName);
        if (pVariant) {
            pResource->SetVariant(*pVariant);
        }
        pResource->SetId(id.ToStringRef(), false /** auto register */);

        if (!pResource->Reload()) {
            SR_ERROR("ResourceManager::LoadResource() : failed to load {}! \n\tPath: {}", typeName, id);
            pResource->DeleteResource();
            return nullptr;
        }

        /// отложенная ручная регистрация
        RegisterResource(pResource);

        return pResource;
    }

    IResource::Ptr ResourceManager::Find(SR_UTILS_NS::StringAtom id, SR_UTILS_NS::StringAtom typeName, const IResourceVariant* pVariant) const {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        if (typeName == Asset::GetClassStaticName()) {
            return FindAnyType(id, pVariant);
        }

        auto&& pIt = m_resources.find(typeName);
        if (pIt == m_resources.end()) {
            return nullptr;
        }

        auto&& [name, resourcesGroup] = *pIt;

        if (auto&& pResource = resourcesGroup->Find(id, pVariant)) {
            /// раз ресурс ищем, значит он все еще может быть нужен.
            pResource->UpdateResourceLifeTime();
            return pResource;
        }

        return nullptr;
    }

    IResource::Ptr ResourceManager::FindAnyType(SR_UTILS_NS::StringAtom id, const IResourceVariant* pVariant) const {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        for (auto&& [typeName, resourcesGroup] : m_resources) {
            if (auto&& pResource = resourcesGroup->Find(id, pVariant)) {
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
                    if (m_destroyQueue.empty()) {
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

    void ResourceManager::ReloadResources(float_t dt) {
        SR_TRACY_ZONE;

        if (m_hasDirtyResources) {
            SR_LOCK_GUARD;
            SR_LOG("ResourceManager::ReloadResources() : reloading dirty resources...");
            for (auto&& [typeName, pResourceType] : m_resources) {
                pResourceType->ReloadDirtyResources();
            }
            m_hasDirtyResources = false;
        }
    }

    void ResourceManager::PullWatchers() {
        SR_TRACY_ZONE;
        m_fileSystemWatcher->WatchPull();
    }

    void ResourceManager::ReloadAll(SR_UTILS_NS::StringAtom typeName) {
        SR_LOCK_GUARD;
        SR_TRACY_ZONE;
        if (auto&& pIt = m_resources.find(typeName); pIt != m_resources.end()) {
            pIt->second->ReloadAll();
        }
        m_hasDirtyResources = true;
    }

    void ResourceManager::ReloadResource(StringAtom id, StringAtom typeName) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        if (auto&& pIt = m_resources.find(typeName); pIt != m_resources.end()) {
            pIt->second->Reload(id);
        }
        else {
            SRHalt("ResourceManager::ReloadResource() : resource type not found!\n\tType: {}", typeName);
        }
        m_hasDirtyResources = true;
    }

    void ResourceManager::ReloadResource(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        if (auto&& pIt = m_resources.find(pResource->GetResourceType()); pIt != m_resources.end()) {
            pIt->second->Reload(pResource);
        }
        else {
            SRHalt("ResourceManager::ReloadResource() : resource type not found!\n\tType: {}", pResource->GetResourceType());
        }
        m_hasDirtyResources = true;
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

        if (!pResource) {
            return false;
        }

        return pResource->Execute([pResource, this](){
            if (!pResource->IsDestroyed()) {
                return true;
            }

            if (pResource->IsAllowedToRevive()) {
                if (auto&& pIt = std::ranges::find(m_destroyQueue, pResource); pIt != m_destroyQueue.end()) {
                    m_destroyQueue.erase(pIt);
                }
                else {
                    SRHalt("ResourceManager::ReviveResource() : resource not found!");
                    return false;
                }

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

    Path ResourceManager::GetEngineCachePath() const {
        if (auto&& cache = SR_PLATFORM_NS::GetApplicationCachePath()) {
            return cache->Concat("Cache");
        }
        return GetEngineResPathRef().Concat("Cache");
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

    void ResourceManager::ChangeResourcesFolder(const Path& path) {
        SR_LOCK_GUARD;
        if (path == m_folder) {
            return;
        }
        SR_LOG("ResourceManager::ChangeResourcesFolder() : changing resources folder to \"{}\"...", path);
        m_folder = path;
        m_fileSystemWatcher.AutoFree();
        m_fileSystemWatcher = FileSystemWatcher::MakeShared();
        m_fileSystemWatcher->AddListener(m_folder);
        m_fileSystemWatcher->StartAsyncWatch();
    }
}