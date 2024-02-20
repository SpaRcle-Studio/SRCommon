// 
// Created by qlop on 19.02.2024 
//

#include <Utils/Network/Client.h>

namespace SR_NETWORK_NS {
    Client::Client(SocketType type, int32_t domain, int32_t service, int32_t protocol, int32_t port, uint64_t interface)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_domain(domain)
        , m_service(service)
        , m_protocol(protocol)
        , m_port(port) 
    {
        // m_socket = Socket::Create(type);
    }

    std::string Client::Request(const std::string& serverIp, void* request, uint64_t size) {
        if (!m_socket->IsOpen()) {
            if (!m_socket->Connect(serverIp, m_port)) {
                SR_ERROR("Client::Request() : failed to connect to server: {}:{}", serverIp, m_port);
                return { };
            }
        }

        if (!m_socket->Send(request, size)) {
            SR_ERROR("Client::Request() : failed to send request to server: {}:{}", serverIp, m_port);
            return { };
        }

        constexpr size_t maxSize = 30000; /// TODO: make it configurable
        std::string response;
        response.reserve(maxSize);

        if (!m_socket->Receive(response.data(), maxSize)) {
            SR_ERROR("Client::Request() : failed to receive response from server: {}:{}", serverIp, m_port);
            return { };
        }

        return response;       
    }
}