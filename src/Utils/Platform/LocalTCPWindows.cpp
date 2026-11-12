//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Network/LocalTCP.h>

#ifdef SR_WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>

namespace SR_NETWORK_NS::LocalTCP {
    Handle Listen(uint16_t& port) {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            return InvalidHandle;
        }
        SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        if (socket == INVALID_SOCKET || bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(socket, 8) != 0) {
            CloseListener(socket);
            return InvalidHandle;
        }
        int32_t size = sizeof(address);
        if (getsockname(socket, reinterpret_cast<sockaddr*>(&address), &size) != 0) {
            CloseListener(socket);
            return InvalidHandle;
        }
        SetHandleInformation(reinterpret_cast<HANDLE>(socket), HANDLE_FLAG_INHERIT, 0);
        port = ntohs(address.sin_port);
        return socket;
    }

    Handle Accept(Handle listener) {
        fd_set pending;
        FD_ZERO(&pending);
        FD_SET(listener, &pending);
        timeval interval{0, 100000};
        if (select(0, &pending, nullptr, nullptr, &interval) <= 0) {
            return InvalidHandle;
        }
        SOCKET client = accept(listener, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            SetHandleInformation(reinterpret_cast<HANDLE>(client), HANDLE_FLAG_INHERIT, 0);
            DWORD timeout = 1000;
            setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
            setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        }
        return client;
    }

    int32_t Receive(Handle socket, char* buffer, uint32_t size) {
        return recv(socket, buffer, static_cast<int32_t>(size), 0);
    }

    int32_t Send(Handle socket, const char* buffer, uint32_t size) {
        return send(socket, buffer, static_cast<int32_t>(size), 0);
    }

    void Close(Handle socket) {
        if (socket != InvalidHandle) {
            closesocket(socket);
        }
    }

    void CloseListener(Handle socket) {
        Close(socket);
        WSACleanup();
    }
}
#endif
