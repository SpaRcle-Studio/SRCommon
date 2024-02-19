// 
// Created by qlop on 19.02.2024 
// 

#ifndef SR_UTILS_NETWORK_CLIENT_H
#define SR_UTILS_NETWORK_CLIENT_H

#include <Utils/Network/Socket.h>
#include <Utils/Network/Server.h>
#include <asio/ip/tcp.hpp>
#include <string>


namespace SR_NETWORK_NS {
    class Client : public SR_HTYPES_NS::SharedPtr<Client> {
        using Super = SR_HTYPES_NS::SharedPtr<Client>;
    public:
        Client(SocketType socket, int32_t domain, int32_t service, int32_t protocol, int32_t port, uint64_t interface);

        std::string Request(std::string_view serverIp, void* request, uint64_t size);

        ~Client();

    private:
        int32_t m_domain;
        int32_t m_service;
        int32_t m_protocol;
        int32_t m_port;
        uint64_t m_interface;

        SR_UTILS_NS::SharedPtr<Socket> m_socket;
    };
}


#endif