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
        bool Start(Callback&& callback) override;
        bool StartAsync(Callback&& callback) override;
        void Stop() override;

    private:
        bool StartBase();

    private:
        std::optional<asio::ip::tcp::acceptor> m_acceptor;
        std::optional<asio::ip::tcp::socket> m_socket;

    };
}

#endif //SR_UTILS_NETWORK_ASIO_TCP_ACCEPTOR_H
