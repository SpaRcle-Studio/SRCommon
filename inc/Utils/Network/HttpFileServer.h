//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_HTTP_FILE_SERVER_H
#define SR_UTILS_HTTP_FILE_SERVER_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/Thread.h>
#include <Utils/Network/LocalTCP.h>

namespace SR_NETWORK_NS {
    class SR_COMMON_DLL_API HttpFileServer : public SR_UTILS_NS::NonCopyable {
    public:
        ~HttpFileServer() override;
        bool Start(const SR_UTILS_NS::Path& root, SR_UTILS_NS::String index = "index.html");
        void Stop();
        SR_UTILS_NS::String GetURL() const;

    private:
        void Serve(const SR_UTILS_NS::Path& root);
        void ServeClient(LocalTCP::Handle client, const SR_UTILS_NS::Path& root);
        bool Send(LocalTCP::Handle client, SR_UTILS_NS::StringView bytes);

        LocalTCP::Handle m_listener = LocalTCP::InvalidHandle;
        SR_HTYPES_NS::Thread::Ptr m_thread = nullptr;
        std::atomic<bool> m_stop = false;
        uint16_t m_port = 0;
        SR_UTILS_NS::String m_index;
    };
}

#endif
