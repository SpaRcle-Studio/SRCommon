//
// Created by Nikita on 16.11.2020.
//

#ifndef SR_ENGINE_IRESOURCE_H
#define SR_ENGINE_IRESOURCE_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Common/SubscriptionHolder.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/Resources/ResourceContainer.h>

namespace SR_UTILS_NS {
    class ResourceManager;
    class ResourceType;
    class FileWatcher;

    struct ResourceInfo;

    struct IResourceVariant {
        virtual ~IResourceVariant() = default;
        SR_NODISCARD virtual SRHashType GetHash() const = 0;
    };

    /// @abstract
    class SR_COMMON_DLL_API IResource : public ResourceContainer, public SubscriptionHolder, public Serializable {
        SR_CLASS()
        friend class ResourceType;
        using ResourceInfoWeakPtr = std::weak_ptr<ResourceInfo>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<IResource>;
        using WeakPtr = SR_HTYPES_NS::WeakPtr<IResource>;

    public:
        SR_INLINE_STATIC const StringAtom RELOAD_BEGIN_EVENT = "ReloadBegin";
        SR_INLINE_STATIC const StringAtom RELOAD_DONE_EVENT = "ReloadDone";

        using Ptr = SR_HTYPES_NS::SharedPtr<IResource>;

        enum class LoadState : uint8_t {
            Unknown, Loaded, Reloading, Loading, Unloading, Unloaded, Error
        };

        enum class RemoveUPResult : uint8_t {
            Delete, Destroy, Success, Error
        };

    public:
        IResource();
        ~IResource() override;

    public:
        bool Execute(const std::function<bool()>& fun) const;

        SR_NODISCARD virtual uint64_t GetFileHash() const;
        SR_NODISCARD virtual bool IsAllowedToRevive() const { return false; }
        SR_NODISCARD virtual Path GetAssociatedPath() const;
        SR_NODISCARD virtual const IResourceVariant* GetVariant() const { return nullptr; }

        SR_NODISCARD bool IsResourceWillBeDeleted() const;
        SR_NODISCARD bool IsRegistered() const noexcept { return m_isRegistered; }
        SR_NODISCARD bool IsLoaded() const noexcept;
        SR_NODISCARD bool IsDestroyed() const noexcept;
        SR_NODISCARD bool IsForceDestroyed() const { return m_isForceDestroyed; }
        SR_NODISCARD bool IsAlive() const { return m_lifetime > 0; }
        SR_NODISCARD uint16_t GetReloadCount() const noexcept;
        SR_NODISCARD uint64_t GetLifetime() const noexcept { return m_lifetime; }
        SR_NODISCARD SR_UTILS_NS::StringAtom GetResourceId() const noexcept;
        SR_NODISCARD SR_UTILS_NS::StringAtom GetResourceType() const noexcept;
        SR_NODISCARD LoadState GetResourceLoadState() const { return m_loadState; }
        SR_NODISCARD uint64_t GetResourceHash() const noexcept { return m_resourceHash; }
        SR_NODISCARD ResourceInfoWeakPtr GetResourceInfo() const noexcept { return m_resourceInfo; }
        SR_NODISCARD bool IsResourceFromMemory() const noexcept { return m_isFromMemory; }

        SR_NODISCARD const SR_UTILS_NS::Path& GetResourcePath() const;
        SR_NODISCARD uint16_t GetCountUses() const noexcept;

        /** Add one point to count uses current resource */
        void AddUsePoint();

        /** Remove one point from count uses current resource */
        virtual RemoveUPResult RemoveUsePoint();

        virtual void CheckResourceUsage();

        virtual void OnResourceRegistered() {
            SRAssert2(!IsRegistered(), "Resource already are registered!");
            m_isRegistered = true;
        }

        virtual void OnReloadDone();

        /** Вызывается только из ResourceManager и IResource, удаляет экземпляр класса,
         * или не удаляет, но это уже не его проблема, а того, как он переопределен.
         * Задача данного метода - сделать финальное оповещение что ресурс не нужен и не отслеживается более. */
        virtual void DeleteResource();

        virtual void StartWatch();
        virtual void StopWatch();

        virtual bool Reload();
        virtual bool Unload();
        virtual bool Load();

        void UpdateResourceLifeTime();
        void SignalWatch();

        /** Call only once | Register resource to destroy in resource manager */
        virtual bool Destroy();
        bool ForceDestroy();
        bool Kill();
        void SetResourceHash(uint64_t hash);
        void SetLifetime(int64_t lifeTime) { m_lifetime = lifeTime; }

        void SetId(SR_UTILS_NS::StringAtom id, bool autoRegister = true);
        void SetId(SR_UTILS_NS::StringAtom id, const SR_UTILS_NS::Path& path, bool autoRegister = true);
        virtual void SetVariant(const IResourceVariant& variant) { }

        virtual void ReviveResource();

    protected:
        std::atomic<LoadState> m_loadState = LoadState::Unknown;

        /// не рекомендуется вручную обращаться к счетчику при наследовании
        std::atomic<uint16_t> m_countUses = 0;

        std::atomic<bool> m_isFromMemory = false;

        std::list<SR_HTYPES_NS::SharedPtr<FileWatcher>> m_watchers;

        std::list<SR_UTILS_NS::StringAtom> m_debugUseStackTraces;
        std::list<SR_UTILS_NS::StringAtom> m_debugUnUseStackTraces;

    private:
        ResourceInfoWeakPtr m_resourceInfo;

        /// хещ-состояние ресурса
        uint64_t m_resourceHash = 0;

        SR_UTILS_NS::StringAtom m_resourceId;
        SR_UTILS_NS::Path m_resourcePath;

        uint16_t m_reloadCount = 0;

        int64_t m_lifetime = 0;

        std::atomic<bool> m_isForceDestroyed = false;
        std::atomic<bool> m_isDestroyed = false;
        std::atomic<bool> m_isRegistered = false;

        /// выставляем при удалении, защита от удаления не через DeleteResource()
        bool m_deleteVerifyFlag = false;

    };
}

#endif //SR_ENGINE_IRESOURCE_H
