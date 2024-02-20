//
// Created by Monika on 19.02.2024.
//

#include <Utils/Common/Enumerations.h>
#include <Utils/Types/SharedPtr.h>

#ifndef SR_UTILS_NETWORK_SOCKET_H
#define SR_UTILS_NETWORK_SOCKET_H

namespace SR_NETWORK_NS {
    SR_ENUM_NS_CLASS_T(SocketType, uint8_t,
        Unknown,
        TCP,
        UDP
    )

    class Socket : public SR_HTYPES_NS::SharedPtr<Socket> {
        using Super = SR_HTYPES_NS::SharedPtr<Socket>;
    protected:
        explicit Socket(SocketType type);

    public:
        virtual ~Socket() = default;

    public:
        SR_NODISCARD static SR_HTYPES_NS::SharedPtr<Socket> Create(SocketType type);

    public:
        SR_NODISCARD virtual bool Connect(const std::string& address, uint16_t port) = 0;
        SR_NODISCARD virtual bool Bind(uint16_t port) = 0;
        SR_NODISCARD virtual bool Listen(int32_t backlog) = 0;
        SR_NODISCARD virtual bool Send(const void* data, size_t size) = 0;
        SR_NODISCARD virtual bool Receive(void* data, size_t size) = 0;
        SR_NODISCARD virtual bool Close() = 0;
        SR_NODISCARD virtual bool IsOpen() const = 0;

        SR_NODISCARD SocketType GetType() const { return m_type; }

    private:
        SocketType m_type = SocketType::Unknown;

    };
}

#endif //SR_UTILS_NETWORK_SOCKET_H
