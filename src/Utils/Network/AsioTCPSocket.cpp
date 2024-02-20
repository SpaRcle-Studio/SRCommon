//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/AsioTCPSocket.h>

namespace SR_NETWORK_NS {
    AsioTCPSocket::AsioTCPSocket()
        : Super(SocketType::TCP)
        , m_context()
        , m_socket(m_context)
    { }

    AsioTCPSocket::~AsioTCPSocket() {
        if (m_socket.is_open()) {
            SR_WARN("~AsioTCPSocket::AsioTCPSocket() : socket is still open, closing it");
            m_socket.close();
        }
    }

    bool AsioTCPSocket::Connect(const std::string& address, uint16_t port) {
        asio::error_code errorCode;
        asio::io_context context;
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address, errorCode), port);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Connect() : failed to create endpoint: {}", errorCode.message());
            return false;
        }

        asio::ip::tcp::socket socket(context);
        socket.connect(endpoint, errorCode);

        if (errorCode) {
            SR_ERROR("AsioTCPSocket::Connect() : failed to connect to address: {}", errorCode.message());
            return false;
        }

        return true;
    }

    bool AsioTCPSocket::Bind(uint16_t port) {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        m_socket.bind(endpoint);
        return true;
    }

    bool AsioTCPSocket::Listen(int32_t backlog) {
       // m_socket.listen(backlog);
        return true;
    }

    bool AsioTCPSocket::Send(const void* data, size_t size) {
        m_socket.send(asio::buffer(data, size));
        return true;
    }

    bool AsioTCPSocket::Receive(void* data, size_t size) {
        m_socket.receive(asio::buffer(data, size));
        return true;
    }

    bool AsioTCPSocket::Close() {
        m_socket.close();
        return true;
    }

    bool AsioTCPSocket::IsOpen() const {
        return m_socket.is_open();
    }
}