//
// Created by Nikita on 17.11.2020.
//

#include <Utils/Resources/IResource.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Resources/FileWatcher.h>
#include <Utils/Platform/Stacktrace.h>

#include <Codegen/IResource.generated.hpp>

namespace SR_UTILS_NS {
    IResource::IResource()
        : ResourceContainer()
        , SubscriptionHolder()
        , Serializable()
        , m_lifetime(ResourceManager::ResourceLifeTime)
    { }

    IResource::~IResource() {
        SRAssert2(GetCountUses() == 0, "Resource has uses!");
        SRAssert2(m_watchers.empty(), "Watchers has not stopped!");
        SRAssert2(m_deleteVerifyFlag || m_loadState == LoadState::Unknown, "DeleteResource() was not called!");
    }

    bool IResource::Reload() {
        SR_TRACY_ZONE;
        SR_TRACY_TEXT_N("Path", GetResourceId());

        if (SR_UTILS_NS::Debug::Instance().GetLevel() >= SR_UTILS_NS::Debug::Level::Low) {
            SR_LOG("IResource::Reload() : reloading \"" + std::string(GetResourceId()) + "\" resource...");
        }

        m_loadState = LoadState::Reloading;

        Broadcast(RELOAD_BEGIN_EVENT);

        Unload();

        if (!Load()) {
            m_loadState = LoadState::Error;
            return false;
        }

        m_loadState = LoadState::Loaded;

        UpdateResources();
        OnReloadDone();

        ++m_reloadCount;

        return true;
    }

    bool IResource::ForceDestroy() {
        if (m_isForceDestroyed || IsDestroyed()) {
            SR_ERROR("IResource::ForceDestroy() : resource is already destroyed!");
            return false;
        }

        m_isForceDestroyed = true;

        return Destroy();
    }

    void IResource::OnReloadDone() {
        Broadcast(RELOAD_DONE_EVENT);
    }

    void IResource::DeleteResource() {
        Unload();
        StopWatch();
        m_deleteVerifyFlag = true;
        GetThis().AutoFree();
    }

    void IResource::SetId(SR_UTILS_NS::StringAtom id, const SR_UTILS_NS::Path& path, bool autoRegister) {
        SRAssert2(!id.empty(), "Invalid id!");

        if (m_resourceId.empty()) {
            SRAssert2(m_resourcePath.empty(), "Resource path already set!");

            m_resourceId = id;
            m_resourcePath = path;

            if (autoRegister) {
                ResourceManager::Instance().RegisterResource(this);
            }
        }
        else {
            SRHalt("Double set resource id!");
        }
    }

    void IResource::SetId(SR_UTILS_NS::StringAtom id, bool autoRegister) {
        SetId(id, SR_UTILS_NS::Path(id), autoRegister);
    }

    void IResource::CheckResourceUsage() {
        ResourceManager::Instance().Execute([this]() {
            if (m_countUses == 0 && !IsDestroyed()) {
                if (IsRegistered()) {
                    Destroy();
                    return;
                }
                else {
                    /// так и не зарегистрировали ресурс
                    DeleteResource();
                    return;
                }
            }
        });
    }

    IResource::RemoveUPResult IResource::RemoveUsePoint() {
        SR_TRACY_ZONE;

        RemoveUPResult result;

        /// тут нужно делать синхронно, иначе может произойти deadlock
        /// TODO: а вообще опасное место, нужно переделать
        /// TODO 2: здесь какой-то пиздец. нужно оптимизировать.
        ResourceManager::Instance().Execute([this, &result]() {
            if (m_countUses == 0) {
                SRHalt("Count use points is zero!");
                result = RemoveUPResult::Error;
                return;
            }

            --m_countUses;

            if (m_countUses == 0 && !IsDestroyed()) {
                if (IsRegistered()) {
                    Destroy();
                    result = RemoveUPResult::Destroy;
                    return;
                }
                else {
                    /// так и не зарегистрировали ресурс
                    DeleteResource();
                    result = RemoveUPResult::Delete;
                    return;
                }
            }

            result = RemoveUPResult::Success;

            /// TODO: получение синглтона дорогая операция, нужно оптимизировать
            if (SR_UTILS_NS::ResourceManager::Instance().IsUsePointStackTraceProfilingEnabled()) {
                m_debugUnUseStackTraces.emplace_back(SR_UTILS_NS::GetStacktrace());
            }
        });

        return result;
    }

    void IResource::AddUsePoint() {
        SR_TRACY_ZONE;

        SRAssert(m_countUses != SR_UINT16_MAX);

        if (m_isRegistered && m_countUses == 0 && m_isDestroyed) {
            SRHalt("IResource::AddUsePoint() : potential multi threading error! Path: " + GetResourcePath().ToStringRef());
        }

        ++m_countUses;

        /// TODO: получение синглтона дорогая операция, нужно оптимизировать
        if (SR_UTILS_NS::ResourceManager::Instance().IsUsePointStackTraceProfilingEnabled()) {
            m_debugUseStackTraces.emplace_back(SR_UTILS_NS::GetStacktrace());
        }
    }

    uint16_t IResource::GetCountUses() const noexcept {
        return m_countUses;
    }

    bool IResource::Destroy() {
        SRAssert(!IsDestroyed());
        m_isDestroyed = true;

        ResourceManager::Instance().Destroy(this);

        return true;
    }

    bool IResource::Kill() {
        if (GetCountUses() == 0) {
            if (!IsDestroyed()) {
                Destroy();
            }

            m_lifetime = 0.f;

            return true;
        }

        return false;
    }

    uint64_t IResource::GetFileHash() const {
        if (IsResourceFromMemory()) {
            return 0;
        }

        SR_TRACY_ZONE;

        auto&& path = Path(GetResourcePath());

        SR_TRACY_TEXT_N("Path", path.ToStringRef());

        if (!path.IsAbs()) {
            path = GetAssociatedPath().Concat(path);
        }

        if (path.Exists(Path::Type::File)) {
            if (auto&& hash = path.GetFileHash(); hash != SR_UINT64_MAX) {
                return hash;
            }
        }

        SRHaltOnce("IResource::GetFileHash() : failed to get resource hash! \n\tResource id: " + std::string(GetResourceId()) +
            "\n\tResource path: " + path.ToString());

        return 0;
    }

    void IResource::SetResourceHash(uint64_t hash) {
        m_resourceHash = hash;
    }

    const SR_UTILS_NS::Path& IResource::GetResourcePath() const {
        return m_resourcePath;
    }

    Path IResource::GetAssociatedPath() const {
        return SR_UTILS_NS::ResourceManager::Instance().GetResPath();
    }

    bool IResource::Unload() {
        if (m_loadState == LoadState::Unknown ||
            m_loadState == LoadState::Loaded ||
            m_loadState == LoadState::Unloading ||
            m_loadState == LoadState::Reloading
        ) {
            m_loadState = LoadState::Unloaded;
            return true;
        }

        for (auto&& pWatch : m_watchers) {
            pWatch->Pause();
        }

        return false;
    }

    bool IResource::Load() {
        if (m_loadState == LoadState::Unknown ||
            m_loadState == LoadState::Unloaded ||
            m_loadState == LoadState::Reloading ||
            m_loadState == LoadState::Loading
        ) {
            m_loadState = LoadState::Loaded;
            return true;
        }

        for (auto&& pWatch : m_watchers) {
            pWatch->Resume();
        }

        return false;
    }

    bool IResource::Execute(const std::function<bool()>& fun) const {
        SR_TRACY_ZONE;
        return fun();
    }

    void IResource::ReviveResource() {
        SRAssert(m_isDestroyed && m_isRegistered);

        m_isDestroyed = false;

        UpdateResourceLifeTime();
    }

    void IResource::UpdateResourceLifeTime() {
        m_lifetime = ResourceManager::ResourceLifeTime;
    }

    void IResource::StopWatch() {
        m_watchers.clear(); /// тут был краш, надо исследовать. крашнуло перезагрузке и удалении вотчера, хотя ресурс корректен.
    }

    void IResource::StartWatch() {
        if (IsResourceFromMemory() || !IsFileResource()) {
            return;
        }

        auto&& resourcesManager = ResourceManager::Instance();

        auto&& path = GetResourcePath();
        auto&& pWatch = SR_UTILS_NS::FileWatcher::MakeShared(resourcesManager.GetResPath().Concat(path));
        if (!pWatch) {
            return;
        }

        pWatch->SetCallBack([this](auto&& pWatcher) {
            SignalWatch();
        });

        m_watchers.emplace_back(pWatch);
    }

    void IResource::SignalWatch() {
        if (IsDestroyed() || IsForceDestroyed()) {
            return;
        }

        ResourceManager::Instance().ReloadResource(this);
    }

    bool IResource::IsResourceWillBeDeleted() const {
        return GetCountUses() == 1 && !IsDestroyed() && !IsRegistered();
    }

    bool IResource::IsLoaded() const noexcept {
        return m_loadState == LoadState::Loaded;
    }

    uint16_t IResource::GetReloadCount() const noexcept {
        return m_reloadCount;
    }

    SR_UTILS_NS::StringAtom IResource::GetResourceId() const noexcept {
        return m_resourceId;
    }

    bool IResource::IsDestroyed() const noexcept {
        return m_isDestroyed;
    }

    SR_UTILS_NS::StringAtom IResource::GetResourceType() const noexcept {
        return GetMeta()->GetFactoryName();
    }
}