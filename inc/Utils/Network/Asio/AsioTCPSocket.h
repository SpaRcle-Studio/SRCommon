//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_ASIO_TCP_SOCKET_H
#define SR_UTILS_NETWORK_ASIO_TCP_SOCKET_H

#include <Utils/Network/Socket.h>

#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/io_context.hpp>

namespace SR_NETWORK_NS {
    class AsioContext;

    class AsioTCPSocket : public Socket {
        using Super = Socket;
        friend class AsioContext;
    private:
        explicit AsioTCPSocket(Context::Ptr pContext);

    public:
        ~AsioTCPSocket() override;

    public:
        bool Connect(const std::string& address, uint16_t port) override;
        bool Listen(int32_t backlog) override;
        bool Send(const void* data, size_t size) override;
        bool Receive(void* data, size_t size) override;
        bool Close() override;
        bool IsOpen() const override;

        SR_NODISCARD std::string GetLocalAddress() const override;
        SR_NODISCARD std::string GetRemoteAddress() const override;

        SR_NODISCARD uint16_t GetLocalPort() const override;
        SR_NODISCARD uint16_t GetRemotePort() const override;

        void SetSocket(asio::ip::tcp::socket&& socket) { m_socket = std::move(socket); }

    private:
        std::optional<asio::ip::tcp::socket> m_socket;

    };
}

#endif //SR_UTILS_NETWORK_ASIO_TCP_SOCKET_H
