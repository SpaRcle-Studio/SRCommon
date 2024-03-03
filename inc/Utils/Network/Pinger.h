//
// Created by innerviewer on 2024-02-20.
//
#ifndef SR_UTILS_NETWORK_PINGER_H
#define SR_UTILS_NETWORK_PINGER_H

#include <Utils/Network/Socket.h>
#include <Utils/Network/Context.h>

namespace SR_NETWORK_NS {
    class Pinger {
    public:
        Pinger() = default;

    public:
        virtual void Ping(const std::string& address) = 0;

    public:
        void SetContext(Context::Ptr context) { m_context = std::move(context); }
        void SetSocket(Socket::Ptr socket) { m_socket = std::move(socket); }

    protected:
        Context::Ptr m_context;
        Socket::Ptr m_socket;
    };
}

#endif //SR_UTILS_NETWORK_PINGER_H
