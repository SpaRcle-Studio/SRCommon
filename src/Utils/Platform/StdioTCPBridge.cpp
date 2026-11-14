//
// Created by Codex on 14.11.2026.
//

#include <Utils/Platform/StdioTCPBridge.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>

#if defined(SR_WIN32)
    #include <winsock2.h>
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#elif defined(SR_LINUX)
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <poll.h>
    #include <unistd.h>
#endif

namespace SR_UTILS_NS::Platform {
    int32_t RunStdioTCPBridge(uint16_t port) {
    #if defined(SR_WIN32) || defined(SR_LINUX)
    #if defined(SR_WIN32)
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            return 1;
        }
        _setmode(_fileno(stdin), _O_BINARY);
        _setmode(_fileno(stdout), _O_BINARY);
        const auto connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connection == INVALID_SOCKET) {
            WSACleanup();
            return 1;
        }
    #else
        const auto connection = socket(AF_INET, SOCK_STREAM, 0);
        if (connection < 0) {
            return 1;
        }
    #endif

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        int32_t result = 0;
        if (connect(connection, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
            SR_PLATFORM_NS::WriteConsoleError("RunStdioTCPBridge() : cannot connect to 127.0.0.1:{}. Start the DAP server first.\n"_format(port));
            result = 1;
        }
        else {
            char buffer[65536];
            bool running = true;
            while (running) {
                bool inputReady = false;
                bool socketReady = false;
            #if defined(SR_WIN32)
                DWORD available = 0;
                if (!PeekNamedPipe(GetStdHandle(STD_INPUT_HANDLE), nullptr, 0, nullptr, &available, nullptr)) {
                    break;
                }
                inputReady = available > 0;
                fd_set readers;
                FD_ZERO(&readers);
                FD_SET(connection, &readers);
                timeval timeout{0, inputReady ? 0 : 10000};
                const auto ready = select(0, &readers, nullptr, nullptr, &timeout);
                socketReady = ready > 0;
            #else
                pollfd descriptors[2] = {{STDIN_FILENO, POLLIN, 0}, {connection, POLLIN, 0}};
                const auto ready = poll(descriptors, 2, 10);
                inputReady = (descriptors[0].revents & (POLLIN | POLLHUP | POLLERR)) != 0;
                socketReady = (descriptors[1].revents & (POLLIN | POLLHUP | POLLERR)) != 0;
            #endif
                if (ready < 0) {
                    result = 1;
                    break;
                }
                if (inputReady) {
                #if defined(SR_WIN32)
                    const auto count = _read(_fileno(stdin), buffer, SR_MIN(available, sizeof(buffer)));
                #else
                    const auto count = read(STDIN_FILENO, buffer, sizeof(buffer));
                #endif
                    if (count <= 0) {
                        break;
                    }
                    int32_t offset = 0;
                    while (offset < count) {
                    #if defined(SR_WIN32)
                        const auto sent = send(connection, buffer + offset, static_cast<int32_t>(count - offset), 0);
                    #else
                        const auto sent = send(connection, buffer + offset, count - offset, MSG_NOSIGNAL);
                    #endif
                        if (sent <= 0) {
                            running = false;
                            result = 1;
                            break;
                        }
                        offset += static_cast<int32_t>(sent);
                    }
                }
                if (running && socketReady) {
                    const auto count = recv(connection, buffer, sizeof(buffer), 0);
                    if (count <= 0) {
                        break;
                    }
                    if (fwrite(buffer, 1, count, stdout) != static_cast<size_t>(count) || fflush(stdout) != 0) {
                        result = 1;
                        break;
                    }
                }
            }
        }

    #if defined(SR_WIN32)
        closesocket(connection);
        WSACleanup();
    #else
        close(connection);
    #endif
        return result;
    #else
        SR_PLATFORM_NS::WriteConsoleError("RunStdioTCPBridge() : unsupported platform.\n");
        return 1;
    #endif
    }
}
