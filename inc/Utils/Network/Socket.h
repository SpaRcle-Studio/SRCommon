//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_SOCKET_H
#define SR_UTILS_NETWORK_SOCKET_H

#include <Utils/Network/Context.h>

namespace SR_NETWORK_NS {
    class Socket : public SR_HTYPES_NS::SharedPtr<Socket> {
        using Super = SR_HTYPES_NS::SharedPtr<Socket>;
    protected:
        explicit Socket(SocketType type, Context::Ptr context)
            : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
            , m_type(type)
            , m_context(std::move(context))
        { };

    public:
        virtual ~Socket() = default;

    public:
        SR_NODISCARD virtual bool Connect(const std::string& address, uint16_t port) = 0;
        SR_NODISCARD virtual bool Listen(int32_t backlog) = 0;
        SR_NODISCARD virtual bool Send(const void* data, size_t size) = 0;
        SR_NODISCARD virtual bool Receive(void* data, size_t size) = 0;
        SR_NODISCARD virtual bool IsOpen() const = 0;

        SR_NODISCARD virtual std::string GetLocalAddress() const = 0;
        SR_NODISCARD virtual std::string GetRemoteAddress() const = 0;

        SR_NODISCARD virtual uint16_t GetLocalPort() const = 0;
        SR_NODISCARD virtual uint16_t GetRemotePort() const = 0;

        virtual bool Close() = 0;

        SR_NODISCARD SocketType GetType() const { return m_type; }

    protected:
        const SocketType m_type = SocketType::Unknown;
        Context::Ptr m_context;

    };
}

#endif //SR_UTILS_NETWORK_SOCKET_H
