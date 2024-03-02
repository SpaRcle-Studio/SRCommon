// 
// Created by qlop on 19.02.2024 
// 

#ifndef SR_UTILS_NETWORK_CLIENT_H
#define SR_UTILS_NETWORK_CLIENT_H

#include <Utils/Network/Socket.h>
#include <Utils/Network/Server.h>

namespace SR_NETWORK_NS {
    class Client : public SR_HTYPES_NS::SharedPtr<Client> {
        using Super = SR_HTYPES_NS::SharedPtr<Client>;
    public:
        Client(SocketType type, int32_t domain, int32_t service, int32_t protocol, int32_t port, uint64_t interface);

    public:
        std::string Request(const std::string& serverIp, void* request, uint64_t size);

        void Ping(const std::string& ip, int32_t port);

    private:
        int32_t m_domain = 0;
        int32_t m_service = 0;
        int32_t m_protocol = 0;
        int32_t m_port = 0;
        uint64_t m_interface = 0;

        SR_HTYPES_NS::SharedPtr<Socket> m_socket;
    };
}

#endif