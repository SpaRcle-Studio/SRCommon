//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/Server.h>

namespace SR_NETWORK_NS {
    Server::Server(SocketType type, int32_t domain, int32_t service, int32_t protocol, uint64_t interface, int32_t port, int32_t backlog)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_domain(domain)
        , m_service(service)
        , m_protocol(protocol)
        , m_port(port)
        , m_backlog(backlog)
        , m_interface(interface)
    {
        m_socket = Socket::Create(type);
    }

    Server::~Server() {
        Stop();
    }

    bool Server::Start() {
        if (m_socket->Bind(m_port)) {
            return m_socket->Listen(m_backlog);
        }
        return false;
    }

    bool Server::Stop() {
        return m_socket->Close();
    }
}