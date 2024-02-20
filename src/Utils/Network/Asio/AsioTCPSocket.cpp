//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/Asio/AsioTCPSocket.h>

namespace SR_NETWORK_NS {
    AsioTCPSocket::AsioTCPSocket(Context::Ptr pContext)
        : Super(SocketType::TCP, std::move(pContext))
    {
        auto&& pAsioContext = m_context.DynamicCast<AsioContext>();
        m_socket = asio::ip::tcp::socket(pAsioContext->GetContext());
    }

    AsioTCPSocket::~AsioTCPSocket() {
        if (m_socket.has_value() && m_socket->is_open()) {
            SR_WARN("~AsioTCPSocket::AsioTCPSocket() : socket is still open, closing it");
            m_socket->close();
            m_socket.reset();
        }
    }

    bool AsioTCPSocket::Connect(const std::string& address, uint16_t port) {
        if (IsOpen()) {
            SR_ERROR("AsioTCPSocket::Connect() : socket is already open!");
            return false;
        }

        asio::error_code errorCode;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address, errorCode), port);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Connect() : failed to create endpoint: {}", errorCode.message());
            return false;
        }

        m_socket->connect(endpoint, errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Connect() : failed to connect to address: {}", errorCode.message());
            return false;
        }

        return true;
    }

    bool AsioTCPSocket::Bind(uint16_t port) {
        if (!m_socket->is_open()) {
            SR_ERROR("AsioTCPSocket::Bind() : socket is not open!");
            return false;
        }

        asio::error_code errorCode;
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        m_socket->bind(endpoint, errorCode);
        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Bind() : failed to bind to port {}: {}", port, errorCode.message());
            return false;
        }

        return true;
    }

    bool AsioTCPSocket::Listen(int32_t backlog) {
       // m_socket.listen(backlog);
        return true;
    }

    bool AsioTCPSocket::Send(const void* data, size_t size) {
        m_socket->send(asio::buffer(data, size));
        return true;
    }

    bool AsioTCPSocket::Receive(void* data, size_t size) {
        m_socket->receive(asio::buffer(data, size));
        return true;
    }

    bool AsioTCPSocket::Close() {
        m_socket->close();
        return true;
    }

    bool AsioTCPSocket::IsOpen() const {
        return m_socket.has_value() && m_socket->is_open();
    }
}