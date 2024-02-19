//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_ASIO_TCP_SOCKET_H
#define SR_UTILS_ASIO_TCP_SOCKET_H

#include <Utils/Network/Socket.h>

#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/io_context.hpp>

namespace SR_NETWORK_NS {
    class AsioTCPSocket : public Socket {
        using Super = Socket;
    public:
        AsioTCPSocket();
        ~AsioTCPSocket() override;

    public:
        bool Connect(const std::string& address, uint16_t port) override;
        bool Bind(uint16_t port) override;
        bool Listen(int32_t backlog) override;
        bool Send(const void* data, size_t size) override;
        bool Receive(void* data, size_t size) override;
        bool Close() override;

    private:
        asio::io_context m_context;
        asio::ip::tcp::socket m_socket;

    };
}

#endif //SR_UTILS_ASIO_TCP_SOCKET_H
