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
            SR_WARN("AsioTCPSocket::~AsioTCPSocket() : socket is still open, closing it");
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
            SR_ERROR("AsioTCPSocket::Connect() : failed to connect to {}:{} address: {}", address, port, errorCode.message());
            Close();
            return false;
        }

        return true;
    }

    bool AsioTCPSocket::Send(const void* data, size_t size) {
        if (size == 0) {
            SR_ERROR("AsioTCPSocket::Send() : invalid size!");
            return false;
        }

        if (!m_socket) {
            SR_ERROR("AsioTCPSocket::Send() : invalid socket!");
            return false;
        }

        if (!data) {
            SR_ERROR("AsioTCPSocket::Send() : invalid data!");
            return false;
        }

        asio::error_code errorCode;
        m_socket->send(asio::buffer(data, size), 0, errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Send() : failed to send data: {}", errorCode.message());
            return false;
        }

        return true;
    }

    uint64_t AsioTCPSocket::Receive(void* data, size_t size) {
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

    bool AsioTCPSocket::Close() {
        if (!m_socket.has_value()) {
            SR_ERROR("AsioTCPSocket::Close() : invalid socket!");
            return false;
        }

        m_socket->close();
        m_socket.reset();

        return true;
    }

    bool AsioTCPSocket::IsOpen() const {
        return m_socket.has_value() && m_socket->is_open();
    }

    std::string AsioTCPSocket::GetLocalAddress() const {
        if (m_socket.has_value()) {
            return m_socket->local_endpoint().address().to_string();
        }

        SR_ERROR("AsioTCPSocket::GetLocalAddress() : invalid socket!");

        return std::string(); /// NOLINT
    }

    uint16_t AsioTCPSocket::GetLocalPort() const {
        if (m_socket.has_value()) {
            return m_socket->local_endpoint().port();
        }

        SR_ERROR("AsioTCPSocket::GetLocalPort() : invalid socket!");

        return 0;
    }

    std::string AsioTCPSocket::GetRemoteAddress() const {
        if (m_socket.has_value()) {
            return m_socket->remote_endpoint().address().to_string();
        }

        SR_ERROR("AsioTCPSocket::GetRemoteAddress() : invalid socket!");

        return std::string(); /// NOLINT
    }

    uint16_t AsioTCPSocket::GetRemotePort() const {
        if (m_socket.has_value()) {
            return m_socket->remote_endpoint().port();
        }

        SR_ERROR("AsioTCPSocket::GetRemotePort() : invalid socket!");

        return 0;
    }

    bool AsioTCPSocket::ReceiveAsyncInternal() {
        if (!m_socket.has_value()) {
            SR_ERROR("AsioTCPSocket::ReceiveAsyncInternal() : invalid socket!");
            return false;
        }

        if (!m_receivedAsyncData) {
            SR_ERROR("AsioTCPSocket::ReceiveAsyncInternal() : invalid received data!");
            return false;
        }

        m_isWaitingReceive = true;

        m_socket->async_receive(asio::buffer(m_receivedAsyncData->GetData(), m_receivedAsyncData->GetSize()),[pStrong = GetThis()](const asio::error_code& errorCode, uint64_t size) {
            pStrong->SetWaitingReceive(false);

            if (errorCode) {
                SR_ERROR("AsioTCPSocket::ReceiveAsyncInternal() : failed to receive data: {}", errorCode.message());
                return;
            }

            if (auto&& pReceiveCallback = pStrong->GetReceiveCallback()) {
                pReceiveCallback(pStrong, pStrong->GetReceivedAsyncData(), size);
            }

            if (pStrong->IsReceiveRepeated()) {
                pStrong->GetContext()->AddAsyncReceiveSocket(pStrong);
            }
        });

        return true;
    }
}