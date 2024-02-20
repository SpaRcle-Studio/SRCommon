//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Asio/AsioTCPAcceptor.h>
#include <Utils/Network/Asio/AsioContext.h>

namespace SR_NETWORK_NS {
    AsioTCPAcceptor::AsioTCPAcceptor(Context::Ptr pContext, std::string address, uint16_t port)
        : Super(SocketType::TCP, std::move(pContext), std::move(address), port)
    { }

    AsioTCPAcceptor::~AsioTCPAcceptor() {
        if (m_acceptor.has_value() && m_acceptor->is_open()) {
            SR_WARN("~AsioTCPAcceptor::AsioTCPAcceptor() : acceptor is still open, closing it.");
            m_acceptor->close();
        }
    }

    bool AsioTCPAcceptor::Start(Callback&& callback) {
        if (m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::Start() : acceptor is already started!");
            return false;
        }

        if (!Super::Start(std::move(callback))) {
            return false;
        }

        asio::error_code errorCode;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(m_address, errorCode), m_port);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::Start() : failed to create endpoint: {}", errorCode.message());
            return false;
        }

        auto&& pAsioContext = m_context.DynamicCast<AsioContext>();

        m_acceptor = asio::ip::tcp::acceptor(pAsioContext->GetContext(), endpoint);
        if (!m_acceptor->is_open()) {
            SR_ERROR("AsioTCPAcceptor::Start() : failed to open acceptor!");
            return false;
        }

        m_socket = asio::ip::tcp::socket(pAsioContext->GetContext());

        m_acceptor->async_accept(m_socket.value(), [this](const asio::error_code& errorCode) {
            if (errorCode) {
                SR_ERROR("AsioTCPAcceptor::Start() : failed to accept connection: {}", errorCode.message());
                return;
            }

            auto&& pAsioContext = m_context.DynamicCast<AsioContext>();
            auto&& pSocket = pAsioContext->CreateSocket(SocketType::TCP);

            pSocket.DynamicCast<AsioTCPSocket>()->SetSocket(std::move(m_socket.value()));

            m_callback(std::move(pSocket));
        });

        return true;
    }

    void AsioTCPAcceptor::Stop() {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::Stop() : acceptor is not started!");
            return;
        }

        m_acceptor->close();
        m_acceptor.reset();

        Super::Stop();
    }
}