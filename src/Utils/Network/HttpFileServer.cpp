#include <Utils/Network/HttpFileServer.h>
#include <Utils/FileSystem/VFS.h>

namespace SR_NETWORK_NS {
    HttpFileServer::~HttpFileServer() {
        Stop();
    }

    bool HttpFileServer::Start(const SR_UTILS_NS::Path& root, SR_UTILS_NS::String index) {
        Stop();
        m_index = std::move(index);
        m_port = 0;
        m_listener = LocalTCP::Listen(m_port);
        if (m_listener == LocalTCP::InvalidHandle) {
            return false;
        }
        m_stop = false;
        return SR_HTYPES_NS::Thread::Factory::Instance().Create(m_thread, [this, root]() {
            Serve(root);
            return false;
        });
    }

    void HttpFileServer::Stop() {
        m_stop = true;
        if (m_thread) {
            m_thread->Join();
            m_thread->Free();
            m_thread = nullptr;
        }
        if (m_listener != LocalTCP::InvalidHandle) {
            LocalTCP::CloseListener(m_listener);
            m_listener = LocalTCP::InvalidHandle;
        }
        m_port = 0;
    }

    SR_UTILS_NS::String HttpFileServer::GetURL() const {
        return m_port ? "http://127.0.0.1:{}/"_format(m_port) : SR_UTILS_NS::String();
    }

    void HttpFileServer::Serve(const SR_UTILS_NS::Path& root) {
        while (!m_stop) {
            const auto client = LocalTCP::Accept(m_listener);
            if (client == LocalTCP::InvalidHandle) {
                continue;
            }
            ServeClient(client, root);
            LocalTCP::Close(client);
        }
    }

    bool HttpFileServer::Send(LocalTCP::Handle client, SR_UTILS_NS::StringView bytes) {
        while (!bytes.empty() && !m_stop) {
            const int32_t sent = LocalTCP::Send(client, bytes.data(), static_cast<uint32_t>(SR_MIN(bytes.size(), 65536)));
            if (sent <= 0) {
                return false;
            }
            bytes.remove_prefix(sent);
        }
        return bytes.empty();
    }

    void HttpFileServer::ServeClient(LocalTCP::Handle client, const SR_UTILS_NS::Path& root) {
        using SR_UTILS_NS::String;
        using SR_UTILS_NS::StringView;
        char buffer[8192];
        uint32_t received = 0;
        while (received < sizeof(buffer) && !m_stop) {
            const int32_t count = LocalTCP::Receive(client, buffer + received, sizeof(buffer) - received);
            if (count <= 0) {
                return;
            }
            received += count;
            if (StringView(buffer, received).contains("\r\n\r\n")) {
                break;
            }
        }
        const StringView request(buffer, received);
        const bool head = request.starts_with("HEAD ");
        const auto begin = request.find(' ');
        const auto end = begin == StringView::npos ? StringView::npos : request.find(' ', begin + 1);
        if ((!head && !request.starts_with("GET ")) || end == StringView::npos) {
            Send(client, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            return;
        }
        auto route = request.substr(begin + 1, end - begin - 1);
        route = route.substr(0, route.find('?'));
        String name(route.starts_with("/") ? route.substr(1) : StringView());
        if (name.empty()) {
            name = m_index;
        }
        if (name.contains("..") || name.contains("\\") || name.contains(":") || name.contains("%") || name.find('\0') != String::npos) {
            Send(client, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            return;
        }
        auto file = SR_UTILS_NS::VFS::Instance().OpenFile(root.Concat(name), SR_UTILS_NS::FileMode::Read);
        if (!file) {
            Send(client, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            return;
        }
        const char* type = name.ends_with(".wasm") ? "application/wasm" : name.ends_with(".js") ? "text/javascript"
            : name.ends_with(".html") ? "text/html; charset=utf-8" : name.ends_with(".json") ? "application/json"
            : name.ends_with(".css") ? "text/css" : name.ends_with(".png") ? "image/png" : "application/octet-stream";
        const String header = "HTTP/1.1 200 OK\r\nContent-Type: {}\r\nContent-Length: {}\r\n"
            "Cross-Origin-Opener-Policy: same-origin\r\nCross-Origin-Embedder-Policy: require-corp\r\n"
            "Cache-Control: no-store\r\nConnection: close\r\n\r\n"_format(type, file.GetSize());
        if (!Send(client, header) || head) {
            return;
        }
        while (!m_stop) {
            const uint64_t count = file.Read(buffer, sizeof(buffer));
            if (!count || !Send(client, StringView(buffer, count))) {
                break;
            }
        }
    }
}

#if !defined(SR_WIN32) && !defined(SR_LINUX)
namespace SR_NETWORK_NS::LocalTCP {
    Handle Listen(uint16_t& port) {
        return InvalidHandle;
    }
    Handle Accept(Handle listener) {
        return InvalidHandle;
    }
    int32_t Receive(Handle socket, char* buffer, uint32_t size) {
        return -1;
    }
    int32_t Send(Handle socket, const char* buffer, uint32_t size) {
        return -1;
    }
    void Close(Handle socket) {
    }
    void CloseListener(Handle socket) {
    }
}
#endif
