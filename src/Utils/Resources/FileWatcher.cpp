//
// Created by Monika on 05.08.2023.
//

#include <Utils/Resources/FileWatcher.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Common/Bind.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Common/StoreUtils.h>
#include <Utils/Common/CLIManager.h>

namespace SR_UTILS_NS {
    FileWatcher::FileWatcher(SR_UTILS_NS::Path path)
        : SR_HTYPES_NS::SharedPtr<FileWatcher>(this, SharedPtrPolicy::Automatic)
        , m_path(std::move(path))
        , m_name("Unnamed")
    {
        if (!StoreUtils::Temp::GetBool("ApplicationHasEmbeddedResources", false)) {
            SR_UTILS_NS::VFS::Instance().ResolvePath(m_path, FileMode::Read);
        }
        else if (CLIManager::Instance().GetProjectPath()) {
            const auto resourcesPath = ResourceManager::Instance().GetResPath();
            if (m_path.ToStringView().starts_with(resourcesPath.ToStringView()) &&
                m_path.size() > resourcesPath.size() && m_path[resourcesPath.size()] == '/') {
                Path physicalResourcesPath = resourcesPath;
                if (VFS::Instance().ResolvePath(physicalResourcesPath, FileMode::Write)) {
                    m_path = physicalResourcesPath.Concat(m_path.ToStringView().substr(resourcesPath.size() + 1));
                }
            }
            else {
                VFS::Instance().ResolvePath(m_path, FileMode::Read);
            }
        }
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
            SR_UTILS_NS::Bind(&FileWatcher::Signal, this, SR_UTILS_NS::Placeholders::_1)
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
