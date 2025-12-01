//
// Created by Monika on 05.08.2023.
//

#include <Utils/Resources/FileWatcher.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Common/SubscriptionMessage.h>

namespace SR_UTILS_NS {
    FileWatcher::FileWatcher(SR_UTILS_NS::Path path)
        : SR_HTYPES_NS::SharedPtr<FileWatcher>(this, SharedPtrPolicy::Automatic)
        , m_path(std::move(path))
        , m_name("Unnamed")
    {
        Subscribe();
    }

    void FileWatcher::SetCallBack(FileWatcher::CallBack callBack) {
        m_callBack = std::move(callBack);
    }

    const SR_UTILS_NS::Path& FileWatcher::GetPath() const noexcept {
        return m_path;
    }

    void FileWatcher::SetName(std::string name) {
        m_name = std::move(name);
    }

    std::string FileWatcher::GetName() const noexcept {
        return m_name;
    }

    void FileWatcher::Subscribe() {
        if (m_subscription.has_value()) {
            return;
        }

        m_subscription = SR_UTILS_NS::ResourceManager::Instance().GetFileSystemWatcher()->Subscribe(
            FileSystemWatcher::MODIFIED_EVENT_ID,
            std::bind(&FileWatcher::Signal, this, std::placeholders::_1)
        );
    }

    void FileWatcher::Unsubscribe() {
        m_subscription.reset();
    }

    void FileWatcher::Signal(const SubscriptionMessage& message) {
        if (message.GetPathRef(FileSystemWatcher::FILE_MSG_ID) != m_path) {
            return;
        }

        if (m_callBack) {
            m_callBack(this);
        }
    }

    void FileWatcher::Pause() {
        Unsubscribe();
    }

    void FileWatcher::Resume() {
        Subscribe();
    }

    FileWatcher::~FileWatcher() = default;
}