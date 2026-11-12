//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Network/LocalTCP.h>

#ifdef SR_LINUX
    #include <sys/socket.h>
    #include <poll.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <fcntl.h>

namespace SR_NETWORK_NS::LocalTCP {
    Handle Listen(uint16_t& port) {
        const int32_t socket = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        if (socket < 0 || bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(socket, 8) != 0) {
            Close(socket);
            return InvalidHandle;
        }
        socklen_t size = sizeof(address);
        if (getsockname(socket, reinterpret_cast<sockaddr*>(&address), &size) != 0) {
            Close(socket);
            return InvalidHandle;
        }
        port = ntohs(address.sin_port);
        return socket;
    }

    Handle Accept(Handle listener) {
        pollfd pending{static_cast<int32_t>(listener), POLLIN, 0};
        if (poll(&pending, 1, 100) <= 0 || !(pending.revents & POLLIN)) {
            return InvalidHandle;
        }
        const int32_t client = accept4(static_cast<int32_t>(listener), nullptr, nullptr, SOCK_CLOEXEC);
        if (client >= 0) {
            timeval timeout{1, 0};
            setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        }
        return client;
    }

    int32_t Receive(Handle socket, char* buffer, uint32_t size) {
        return static_cast<int32_t>(recv(static_cast<int32_t>(socket), buffer, size, 0));
    }

    int32_t Send(Handle socket, const char* buffer, uint32_t size) {
        return static_cast<int32_t>(send(static_cast<int32_t>(socket), buffer, size, MSG_NOSIGNAL));
    }

    void Close(Handle socket) {
        if (socket != InvalidHandle) {
            close(static_cast<int32_t>(socket));
        }
    }

    void CloseListener(Handle socket) {
        Close(socket);
    }
}
#endif
