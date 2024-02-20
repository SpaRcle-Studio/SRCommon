// 
// Created by qlop on 19.02.2024 
//

#include <Utils/Network/Client.h>

#include <asio/ip/tcp.hpp>

namespace SR_NETWORK_NS {
    Client::Client(SocketType type, int32_t domain, int32_t service, int32_t protocol, int32_t port, uint64_t interface)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_domain(domain)
        , m_service(service)
        , m_protocol(protocol)
        , m_port(port) 
    {
        m_socket = Socket::Create(type);
    }


    std::string Client::Request(const std::string& serverIp, void* request, uint64_t size) {
        asio::io_context ioContext;
        asio::ip::tcp::endpoint servEndpoint(asio::ip::address::from_string(serverIp), m_port);
        
        asio::ip::tcp::socket socket(ioContext);
        socket.connect(servEndpoint);


        socket.send(asio::buffer(request, size));

        constexpr size_t maxSize = 30000;
        std::string response;
        response.reserve(maxSize);
        socket.receive(asio::buffer(response.data(), maxSize));

        return response;       
    }


}