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
            SR_WARN("AsioTCPAcceptor::~AsioTCPAcceptor() : acceptor is still open, closing it.");
            m_acceptor->close();
        }
    }

    bool AsioTCPAcceptor::StartInternal(bool async) {
        if (IsWaitingAccept()) {
            SRHalt("AsioTCPAcceptor::StartInternal() : acceptor is already waiting for accept!");
            return false;
        }

        if (!Init()) {
            SR_ERROR("AsioTCPAcceptor::StartInternal() : failed to init acceptor!");
            return false;
        }

        if (!m_socket.has_value()) {
            m_socket = asio::ip::tcp::socket(m_context.DynamicCast<AsioContext>()->GetContext());
        }

        if (async) {
            m_isWaitingAccept = true;
            m_acceptor->async_accept(m_socket.value(), [this](const asio::error_code& errorCode) {
                m_isWaitingAccept = false;
                Accept(errorCode);
            });
            return true;
        }

        asio::error_code errorCode;

        if (m_isRepeated) {
            while (m_callback && IsOpen()) {
                m_acceptor->accept(m_socket.value(), errorCode);
                if (!Accept(errorCode)) {
                    return false;
                }
            }
            return true;
        }

        m_acceptor->accept(m_socket.value(), errorCode);
        return Accept(errorCode);
    }

    void AsioTCPAcceptor::Close() {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::Close() : acceptor is not started!");
            return;
        }

        m_acceptor->close();
        m_acceptor.reset();

        Super::Close();
    }

    bool AsioTCPAcceptor::Init() {
        if (!IsOpen()) {
            SR_ERROR("AsioTCPAcceptor::Init() : acceptor is not open!");
            return false;
        }

        if (m_acceptor.has_value() && m_acceptor->is_open()) {
            return true;
        }

        asio::error_code errorCode;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(m_address, errorCode), m_port);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::Init() : failed to create endpoint: {}", errorCode.message());
            return false;
        }

        auto&& pAsioContext = m_context.DynamicCast<AsioContext>();

        m_acceptor = asio::ip::tcp::acceptor(pAsioContext->GetContext(), endpoint);
        if (!m_acceptor->is_open()) {
            SR_ERROR("AsioTCPAcceptor::Init() : failed to open acceptor!");
            return false;
        }

        return true;
    }

    std::string AsioTCPAcceptor::GetLocalAddress() const {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::GetLocalAddress() : acceptor is not started!");
            return {};
        }

        asio::error_code errorCode;
        auto&& endpoint = m_acceptor->local_endpoint(errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::GetLocalAddress() : failed to get local endpoint: {}", errorCode.message());
            return {};
        }

        return endpoint.address().to_string();
    }

    std::string AsioTCPAcceptor::GetRemoteAddress() const {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::GetRemoteAddress() : acceptor is not started!");
            return {};
        }

        asio::error_code errorCode;
        auto&& endpoint = m_acceptor->local_endpoint(errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::GetRemoteAddress() : failed to get local endpoint: {}", errorCode.message());
            return {};
        }

        return endpoint.address().to_string();
    }

    uint16_t AsioTCPAcceptor::GetLocalPort() const {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::GetLocalPort() : acceptor is not started!");
            return 0;
        }

        asio::error_code errorCode;
        auto&& endpoint = m_acceptor->local_endpoint(errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::GetLocalPort() : failed to get local endpoint: {}", errorCode.message());
            return 0;
        }

        return endpoint.port();
    }

    uint16_t AsioTCPAcceptor::GetRemotePort() const {
        if (!m_acceptor.has_value()) {
            SR_ERROR("AsioTCPAcceptor::GetRemotePort() : acceptor is not started!");
            return 0;
        }

        asio::error_code errorCode;
        auto&& endpoint = m_acceptor->local_endpoint(errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::GetRemotePort() : failed to get local endpoint: {}", errorCode.message());
            return 0;
        }

        return endpoint.port();
    }

    bool AsioTCPAcceptor::Accept(const asio::error_code& errorCode) {
        if (errorCode) {
            SR_ERROR("AsioTCPAcceptor::Start() : failed to accept connection: {}", errorCode.message());
            return false;
        }

        auto&& pAsioContext = m_context.DynamicCast<AsioContext>();
        auto&& pSocket = pAsioContext->CreateSocket(SocketType::TCP);

        pSocket.DynamicCast<AsioTCPSocket>()->SetSocket(std::move(m_socket.value()));
        m_socket.reset();

        if (IsOpen() && IsRepeated()) {
            m_context->AddAsyncAcceptor(GetThis());
        }

        if (m_callback) {
            m_callback(std::move(pSocket));
            return true;
        }

        SR_ERROR("AsioTCPAcceptor::Accept() : callback is not set!");
        pSocket->Close();

        return false;
    }
}