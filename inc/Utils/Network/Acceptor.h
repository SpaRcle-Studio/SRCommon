//
// Created by Monika on 20.02.2024.
//

#ifndef SR_UTILS_NETWORK_ACCEPTOR_H
#define SR_UTILS_NETWORK_ACCEPTOR_H

#include <Utils/Network/Context.h>

namespace SR_NETWORK_NS {
    class Acceptor : public SR_HTYPES_NS::SharedPtr<Acceptor> {
        friend class Context;
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

        SR_NODISCARD virtual std::string GetLocalAddress() const = 0;
        SR_NODISCARD virtual std::string GetRemoteAddress() const = 0;

        SR_NODISCARD virtual uint16_t GetLocalPort() const = 0;
        SR_NODISCARD virtual uint16_t GetRemotePort() const = 0;

        SR_NODISCARD virtual bool Init() = 0;

        SR_NODISCARD bool IsOpen() const { return m_isOpen; }
        SR_NODISCARD bool IsRepeated() const { return m_isRepeated; }
        SR_NODISCARD bool IsWaitingAccept() const { return m_isWaitingAccept; }

        void SetCallback(Callback&& callback) { m_callback = std::move(callback); }
        void SetRepeated(bool isRepeated) { m_isRepeated = isRepeated; }

        bool Start();
        bool StartAsync();

        virtual void Close() {
            m_isOpen = false;
        }

    protected:
        virtual bool StartInternal(bool async) = 0;

    protected:
        bool m_isOpen = false;
        bool m_isWaitingAccept = false;

        /// Нужно ли повторять прослушивание новых соединений.
        /// Работает только для асинхронного режима.
        bool m_isRepeated = true;

        const SocketType m_type = SocketType::Unknown;

        Context::Ptr m_context;
        Callback m_callback;
        std::string m_address;
        uint16_t m_port = 0;

    };
}

#endif //SR_UTILS_NETWORK_ACCEPTOR_H
