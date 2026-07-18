//
// Created by Monika on 03.04.2025.
//

#include <Utils/Resources/FileSystemWatcher.h>
#include <Utils/Common/Features.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Types/Thread.h>

#ifdef SR_EFSW_USE
    #include <efsw/efsw.hpp>
#endif

namespace SR_UTILS_NS {
    class FileSystemWatcher;

#ifdef SR_EFSW_USE
    class FileSystemUpdateListener : public efsw::FileWatchListener, SR_UTILS_NS::NonCopyable {
    public:
        FileSystemUpdateListener(FileSystemWatcher* pWatcher, Path path)
            : m_pWatcher(pWatcher)
            , m_path(std::move(path))
        { }

    public:
        void handleFileAction(efsw::WatchID watchId, const std::string& dir, const std::string& filename, efsw::Action action, const std::string& oldFilename) override {
            SR_TRACY_ZONE;

            m_pWatcher->Lock();

            FileSystemWatcher::Event& event = m_pWatcher->AddEmptyEvent();
            event.oldFilename = oldFilename;
            event.filename = filename;
            event.dir = dir;

            switch (action) {
                case efsw::Actions::Add:
                    event.type = FileSystemWatcher::EventType::Add;
                    break;
                case efsw::Actions::Delete:
                    event.type = FileSystemWatcher::EventType::Delete;
                    break;
                case efsw::Actions::Modified:
                    event.type = FileSystemWatcher::EventType::Modified;
                    break;
                case efsw::Actions::Moved:
                    event.type = FileSystemWatcher::EventType::Move;
                    break;
                default:
                    SRHalt("FileSystemWatcher::handleFileAction() : unknown action!");
                    break;
            }

            m_pWatcher->Unlock();
        }

    private:
        FileSystemWatcher* m_pWatcher = nullptr;
        Path m_path;

    };
#endif

    FileSystemWatcher::FileSystemWatcher()
        : Super(this, SharedPtrPolicy::Automatic)
    {
    #ifdef SR_EFSW_USE
        m_pImpl = new efsw::FileWatcher();
    #endif
        m_messageCache = new SR_UTILS_NS::SubscriptionMessage();
        m_events.resize(64);
    }

    FileSystemWatcher::~FileSystemWatcher() {
    #ifdef SR_EFSW_USE
        delete static_cast<efsw::FileWatcher*>(m_pImpl);
    #endif
    }

    void FileSystemWatcher::StartAsyncWatch() {
        SR_TRACY_ZONE;

        if (m_isAsyncWatchingActive) {
            SRHalt("FileSystemWatcher::AsyncWatch() : already watching!");
            return;
        }
        m_isAsyncWatchingActive = true;

        if (!SR_UTILS_NS::Features::Instance().Enabled("FileWatching", true)) {
            SR_INFO("FileSystemWatcher::AsyncWatch() : file watching is disabled!");
            return;
        }

    #ifdef SR_EFSW_USE
        static_cast<efsw::FileWatcher*>(m_pImpl)->watch();
    #endif
    }

    void FileSystemWatcher::Lock() {
        SR_TRACY_ZONE;
        m_mutex.lock();
    }

    void FileSystemWatcher::Unlock()  {
        SR_TRACY_ZONE;
        m_mutex.unlock();
    }

    void FileSystemWatcher::WatchPull() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (size_t i = 0; i < m_usedEvents; ++i) {
            auto&& event = m_events[i];

            m_pathTmp = event.dir;
            m_messageCache->SetPath(DIR_MSG_ID, m_pathTmp);

            m_pathTmp.GetInternalUnsafeString() = event.dir;
            m_pathTmp.GetInternalUnsafeString().append(event.filename);
            m_pathTmp.Normalize();
            m_messageCache->SetPath(FILE_MSG_ID, m_pathTmp);

            m_pathTmp.GetInternalUnsafeString() = event.dir;
            m_pathTmp.GetInternalUnsafeString().append(event.oldFilename);
            m_pathTmp.Normalize();
            m_messageCache->SetPath(OLD_FILE_MSG_ID, m_pathTmp);

            switch (event.type) {
                case EventType::Add:
                    Broadcast(ADDED_EVENT_ID, *m_messageCache);
                    break;
                case EventType::Delete:
                    Broadcast(DELETED_EVENT_ID, *m_messageCache);
                    break;
                case EventType::Move:
                    Broadcast(MOVED_EVENT_ID, *m_messageCache);
                    break;
                case EventType::Modified:
                    Broadcast(MODIFIED_EVENT_ID, *m_messageCache);
                    break;
                default:
                    SRHalt("FileSystemWatcher::WatchPull() : unknown event type!");
                    break;
            }
        }

        m_usedEvents = 0;
    }

    void FileSystemWatcher::AddListener(const Path& path) {
        SR_TRACY_ZONE;

    #ifdef SR_EFSW_USE
        auto&& pListener = new FileSystemUpdateListener(this, path);
        const efsw::WatchID id = static_cast<efsw::FileWatcher*>(m_pImpl)->addWatch(path.ToString(), pListener, true);
        if (m_listeners.find(id) != m_listeners.end()) {
            SRHalt("FileSystemWatcher::AddListener() : listener already exists!");
        }
        m_listeners[id] = pListener;
    #endif
    }

    void FileSystemWatcher::OnEvent(FileSystemWatcher::Event&& event) {
        SR_LOCK_GUARD;
        AddEmptyEvent() = std::move(event);
    }

    FileSystemWatcher::Event& FileSystemWatcher::AddEmptyEvent() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        if (m_usedEvents >= m_events.size()) {
            m_events.emplace_back();
        }
        return m_events[m_usedEvents++];
    }
}