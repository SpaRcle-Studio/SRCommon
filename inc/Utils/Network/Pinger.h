//
// Created by innerviewer on 2024-02-20.
//
#ifndef SR_UTILS_NETWORK_PINGER_H
#define SR_UTILS_NETWORK_PINGER_H

namespace SR_NETWORK_NS {
    class Pinger {
    public:
        Pinger() = default;

    public:
        virtual void Ping() = 0;

    protected:
        Context::Ptr m_context;
        Socket::Ptr m_socket;
    };
}

#endif //SR_UTILS_NETWORK_PINGER_H
