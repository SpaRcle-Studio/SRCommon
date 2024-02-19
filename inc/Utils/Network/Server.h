//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_SERVER_H
#define SR_UTILS_NETWORK_SERVER_H

#include <Utils/Network/Socket.h>
#include <Utils/Types/NodeDictionary.h>

namespace SR_NETWORK_NS {
    class Server : public SR_HTYPES_NS::SharedPtr<Server> {
        using Super = SR_HTYPES_NS::SharedPtr<Server>;
    public:
        Server(SocketType type, int32_t domain, int32_t service, int32_t protocol, uint64_t interface, int32_t port, int32_t backlog);
        ~Server();

    public:
        bool Start();
        bool Stop();

    private:
        int32_t m_domain = 0;
        int32_t m_service = 0;
        int32_t m_protocol = 0;
        int32_t m_port = 0;
        int32_t m_backlog = 0;
        uint64_t m_interface = 0;

        /// SR_HTYPES_NS::NodeDictionary<void> m_routes;

        SR_HTYPES_NS::SharedPtr<Socket> m_socket;

    };
}

#endif //SR_UTILS_NETWORK_SERVER_H
