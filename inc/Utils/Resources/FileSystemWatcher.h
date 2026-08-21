//
// Created by Monika on 03.04.2025.
//

#ifndef SR_ENGINE_UTILS_DIRECTORY_WATCHER_H
#define SR_ENGINE_UTILS_DIRECTORY_WATCHER_H

#include <Utils/Types/SharedPtr.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/RawPointerHolder.h>
#include <Utils/Common/SubscriptionHolder.h>

namespace SR_UTILS_NS {
    class FileSystemWatcher final : public SR_HTYPES_NS::SharedPtr<FileSystemWatcher>, public SubscriptionHolder {
        using Super = SR_HTYPES_NS::SharedPtr<FileSystemWatcher>;
    public:
        static const inline StringAtom MODIFIED_EVENT_ID = "Modified";
        static const inline StringAtom ADDED_EVENT_ID = "Added";
        static const inline StringAtom DELETED_EVENT_ID = "Deleted";
        static const inline StringAtom MOVED_EVENT_ID = "Moved";

        static const inline StringAtom DIR_MSG_ID = "Directory";
        static const inline StringAtom FILE_MSG_ID = "File";
        static const inline StringAtom OLD_FILE_MSG_ID = "OldFile";

        enum class EventType : uint8_t {
            None, Add, Delete, Move, Modified
        };
        struct Event {
            EventType type = EventType::None;
            String dir;
            String filename;
            String oldFilename;
        };

    public:
        FileSystemWatcher();
        ~FileSystemWatcher() override;

    public:
        void StartAsyncWatch();
        void WatchPull();

        void Lock();
        void Unlock();

        void AddListener(const Path& path);

        void OnEvent(Event&& event);
        Event& AddEmptyEvent();

    private:
        RawPointerHolder<SubscriptionMessage> m_messageCache;
        void* m_pImpl = nullptr;
        Map<uint64_t, void*> m_listeners;
        bool m_isAsyncWatchingActive = false;
        std::recursive_mutex m_mutex;
        uint32_t m_usedEvents = 0;
        Vector<Event> m_events;
        Path m_pathTmp;

    };
}

#endif //SR_ENGINE_UTILS_DIRECTORY_WATCHER_H
