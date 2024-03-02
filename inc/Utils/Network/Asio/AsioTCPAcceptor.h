//
// Created by Monika on 20.02.2024.
//

#ifndef SR_UTILS_NETWORK_ASIO_TCP_ACCEPTOR_H
#define SR_UTILS_NETWORK_ASIO_TCP_ACCEPTOR_H

#include <Utils/Network/Acceptor.h>

#include <asio/ip/tcp.hpp>
#include <asio/io_service.hpp>

namespace SR_NETWORK_NS {
    class AsioContext;

    class AsioTCPAcceptor : public Acceptor {
        using Super = Acceptor;
        friend class AsioContext;
    private:
        AsioTCPAcceptor(Context::Ptr pContext, std::string address, uint16_t port);

    public:
        ~AsioTCPAcceptor() override;

    public:
        void Close() override;

        SR_NODISCARD std::string GetLocalAddress() const override;
        SR_NODISCARD std::string GetRemoteAddress() const override;

        SR_NODISCARD uint16_t GetLocalPort() const override;
        SR_NODISCARD uint16_t GetRemotePort() const override;

    private:
        bool Init() override;

        bool StartInternal(bool async) override;
        bool Accept(const asio::error_code& errorCode);

    private:
        std::optional<asio::ip::tcp::acceptor> m_acceptor;
        std::optional<asio::ip::tcp::socket> m_socket;

    };
}

#endif //SR_UTILS_NETWORK_ASIO_TCP_ACCEPTOR_H
