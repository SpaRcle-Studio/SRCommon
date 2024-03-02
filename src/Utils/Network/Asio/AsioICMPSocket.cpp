//
// Created by innerviewer on 2024-02-20.
//

#include <Utils/Network/Asio/AsioICMPSocket.h>

namespace SR_NETWORK_NS {
    AsioICMPSocket::AsioICMPSocket(Context::Ptr pContext)
            : Super(SocketType::ICMP, std::move(pContext))
    {
        auto&& pAsioContext = m_context.DynamicCast<AsioContext>();
        m_socket = asio::ip::icmp::socket(pAsioContext->GetContext(), asio::ip::icmp::v4());
    }

    AsioICMPSocket::~AsioICMPSocket() {
        if (m_socket.has_value() && m_socket->is_open()) {
            SR_WARN("AsioICMPSocket::~AsioICMPSocket() : socket is still open, closing it");
            m_socket->close();
            m_socket.reset();
        }
    }

    bool AsioICMPSocket::Connect(const std::string& address, uint16_t port) {
        if (IsOpen()) {
            SR_ERROR("AsioICMPSocket::Connect() : socket is already open!");
            return false;
        }

        asio::error_code errorCode;
        asio::ip::icmp::endpoint endpoint(asio::ip::make_address(address, errorCode), port);

        if (errorCode) {
            SR_ERROR("AsioICMPSocket::Connect() : failed to create endpoint: {}", errorCode.message());
            return false;
        }

        m_socket->connect(endpoint, errorCode);

        if (errorCode) {
            SR_ERROR("AsioICMPSocket::Connect() : failed to connect to address: {}", errorCode.message());
            return false;
        }

        return true;
    }

    bool AsioICMPSocket::Bind(uint16_t port) {
        if (!m_socket->is_open()) {
            SR_ERROR("AsioICMPSocket::Bind() : socket is not open!");
            return false;
        }

        asio::error_code errorCode;
        asio::ip::icmp::endpoint endpoint(asio::ip::icmp::v4(), port);
        m_socket->bind(endpoint, errorCode);
        if (errorCode) {
            SR_ERROR("AsioICMPSocket::Bind() : failed to bind to port {}: {}", port, errorCode.message());
            return false;
        }

        return true;
    }

    bool AsioICMPSocket::Listen(int32_t backlog) {
        // m_socket.listen(backlog);
        return true;
    }

    bool AsioICMPSocket::Send(const void* data, size_t size) {
        m_socket->send(asio::buffer(data, size));
        return true;
    }

    uint64_t AsioICMPSocket::Receive(void* data, size_t size) {
        if (size == 0) {
            SR_ERROR("AsioTCPSocket::Receive() : invalid size!");
            return 0;
        }

        if (!m_socket) {
            SR_ERROR("AsioTCPSocket::Receive() : invalid socket!");
            return 0;
        }

        asio::error_code errorCode;
        const uint64_t receivedSize = m_socket->receive(asio::buffer(data, size), 0, errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Receive() : failed to receive data: {}", errorCode.message());
            return 0;
        }

        return receivedSize;
    }

    bool AsioICMPSocket::Close() {
        m_socket->close();
        return true;
    }

    bool AsioICMPSocket::IsOpen() const {
        return m_socket.has_value() && m_socket->is_open();
    }

    std::string AsioICMPSocket::GetLocalAddress() const {
        return std::string();
    }

    std::string AsioICMPSocket::GetRemoteAddress() const {
        return std::string();
    }

    uint16_t AsioICMPSocket::GetLocalPort() const {
        return 0;
    }

    uint16_t AsioICMPSocket::GetRemotePort() const {
        return 0;
    }

    bool AsioICMPSocket::ReceiveAsyncInternal() {
        return false;
    }
}