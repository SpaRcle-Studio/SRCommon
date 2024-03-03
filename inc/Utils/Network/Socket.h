//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_SOCKET_H
#define SR_UTILS_NETWORK_SOCKET_H

#include <Utils/Network/Context.h>

namespace SR_NETWORK_NS {
    class Socket : public SR_HTYPES_NS::SharedPtr<Socket> {
        friend class Context;
        using Super = SR_HTYPES_NS::SharedPtr<Socket>;
        using ReceiveCallback = SR_HTYPES_NS::Function<void(const Socket::Ptr&, const DataPackage::Ptr&, uint64_t size)>;
    protected:
        explicit Socket(SocketType type, Context::Ptr context);

    public:
        virtual ~Socket() = default;

    public:
        void SetReceiveCallback(ReceiveCallback&& callback) { m_receiveCallback = std::move(callback); }

        SR_NODISCARD DataPackage::Ptr Receive(uint64_t size);
        bool AsyncReceive(uint64_t size);

        virtual bool Send(const void* data, size_t size) = 0;
        virtual bool SendTo(const void* data, uint64_t size, const std::string& address, uint16_t port) = 0;
        SR_NODISCARD virtual uint64_t Receive(void* data, size_t size) = 0;
        SR_NODISCARD virtual uint64_t AsyncReceive(void* data, std::function<void(uint64_t bytesReceived)> callback) = 0;
        SR_NODISCARD virtual bool Connect(const std::string& address, uint16_t port) = 0;
        SR_NODISCARD virtual bool IsOpen() const = 0;

        SR_NODISCARD virtual std::string GetLocalAddress() const = 0;
        SR_NODISCARD virtual std::string GetRemoteAddress() const = 0;

        SR_NODISCARD virtual uint16_t GetLocalPort() const = 0;
        SR_NODISCARD virtual uint16_t GetRemotePort() const = 0;

        virtual bool Close() = 0;

        SR_NODISCARD SocketType GetType() const { return m_type; }
        SR_NODISCARD bool IsReceiveRepeated() const { return m_isReceiveRepeated; }
        SR_NODISCARD const DataPackage::Ptr& GetReceivedData() const { return m_receivedData; }
        SR_NODISCARD const DataPackage::Ptr& GetReceivedAsyncData() const { return m_receivedAsyncData; }
        SR_NODISCARD const ReceiveCallback& GetReceiveCallback() const { return m_receiveCallback; }
        SR_NODISCARD const Context::Ptr& GetContext() const { return m_context; }

        void SetWaitingReceive(bool isWaiting) { m_isWaitingReceive = isWaiting; }

    protected:
        virtual bool ReceiveAsyncInternal() = 0;

    protected:
        const SocketType m_type = SocketType::Unknown;
        Context::Ptr m_context;

        ReceiveCallback m_receiveCallback;

        DataPackage::Ptr m_receivedAsyncData;
        DataPackage::Ptr m_receivedData;

        bool m_isReceiveRepeated = true;
        bool m_isWaitingReceive = false;

    };
}

#endif //SR_UTILS_NETWORK_SOCKET_H
