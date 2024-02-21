//
// Created by innerviewer on 2024-02-20.
//

#ifndef SR_UTILS_NETWORK_ASIOICMPSOCKET_H
#define SR_UTILS_NETWORK_ASIOICMPSOCKET_H

#include <Utils/Network/Socket.h>

#include <asio/ip/icmp.hpp>
#include <asio/io_context.hpp>

namespace SR_NETWORK_NS {
    class AsioContext;

    class AsioICMPSocket : public Socket {
        using Super = Socket;
        friend class AsioContext;
    private:
        explicit AsioICMPSocket(Context::Ptr pContext);

    public:
        ~AsioICMPSocket() override;

    public:
        bool Connect(const std::string& address, uint16_t port) override;
        bool Bind(uint16_t port) override;
        bool Listen(int32_t backlog) override;
        bool Send(const void* data, size_t size) override;
        bool Receive(void* data, size_t size) override;
        bool Close() override;
        bool IsOpen() const override;

        void SetSocket(asio::ip::icmp::socket&& socket) { m_socket = std::move(socket); }

    private:
        std::optional<asio::ip::icmp::socket> m_socket;

    };
}

#endif //SR_UTILS_NETWORK_ASIOICMPSOCKET_H
