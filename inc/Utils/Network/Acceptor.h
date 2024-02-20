//
// Created by Monika on 20.02.2024.
//

#ifndef SR_UTILS_NETWORK_ACCEPTOR_H
#define SR_UTILS_NETWORK_ACCEPTOR_H

#include <Utils/Network/Context.h>

namespace SR_NETWORK_NS {
    class Acceptor : public SR_HTYPES_NS::SharedPtr<Acceptor> {
        using Super = SR_HTYPES_NS::SharedPtr<Acceptor>;
    public:
        using Callback = std::function<void(Socket::Ptr)>;

    protected:
        explicit Acceptor(SocketType type, Context::Ptr pContext, std::string address, uint16_t port)
            : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
            , m_type(type)
            , m_context(std::move(pContext))
            , m_address(std::move(address))
            , m_port(port)
        { }

    public:
        virtual ~Acceptor() = default;

        virtual bool Start(Callback&& callback) {
            m_callback = std::move(callback);
            return true;
        }

        virtual void Stop() { m_callback = nullptr; }

    protected:
        const SocketType m_type = SocketType::Unknown;
        Context::Ptr m_context;
        Callback m_callback;
        std::string m_address;
        uint16_t m_port = 0;

    };
}

#endif //SR_UTILS_NETWORK_ACCEPTOR_H
