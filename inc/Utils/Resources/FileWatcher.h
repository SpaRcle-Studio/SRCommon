//
// Created by Monika on 05.08.2023.
//

#ifndef SR_ENGINE_FILEWATCHER_H
#define SR_ENGINE_FILEWATCHER_H

#include <Utils/Debug.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/SubscriptionHolder.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    class ResourceManager;

    class SR_DLL_EXPORT FileWatcher final : public SR_HTYPES_NS::SharedPtr<FileWatcher>, SR_UTILS_NS::NonCopyable {
        using CallBack = SR_HTYPES_NS::Function<void(FileWatcher* pWatcher)>;
        friend class ResourceManager;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<FileWatcher>;

    public:
        explicit FileWatcher(SR_UTILS_NS::Path path);

    public:
        void SetCallBack(CallBack callBack);
        void SetName(std::string name);

        void Pause() { Unsubscribe(); }
        void Resume() { Subscribe(); }

        SR_NODISCARD std::string GetName() const noexcept;
        SR_NODISCARD const SR_UTILS_NS::Path& GetPath() const noexcept;

    private:
        void Subscribe();
        void Unsubscribe();

        void Signal(const SR_UTILS_NS::SubscriptionMessage& message);

    private:
        std::optional<Subscription> m_subscription;
        SR_UTILS_NS::Path m_path;
        CallBack m_callBack;
        std::string m_name;

    };
}

#endif //SR_ENGINE_FILEWATCHER_H
