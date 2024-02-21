//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_PEER_TO_PEER_H
#define SR_UTILS_NETWORK_PEER_TO_PEER_H

#include <Utils/Network/Socket.h>
#include <Utils/Network/Acceptor.h>

namespace SR_NETWORK_NS {
    class Context;

    class PeerToPeer : public SR_HTYPES_NS::SharedPtr<PeerToPeer> {
        friend class Context;
        using Super = SR_HTYPES_NS::SharedPtr<PeerToPeer>;
        using Callback = std::function<void(const PeerToPeer::Ptr& pP2P, const Socket::Ptr&)>;
    private:
        PeerToPeer(SocketType type, Context::Ptr pContext, std::string address, uint16_t port);

    public:
        virtual ~PeerToPeer() = default;

    public:
        bool Run();
        void Close();

        bool Connect(const std::string& address, uint16_t port);

        void SetOnAcceptCallback(Callback&& callback) { m_onAccept = std::move(callback); }

        SR_NODISCARD const Acceptor::Ptr& GetAcceptor() const { return m_acceptor; }

    private:
        void OnAccept(Socket::Ptr&& pSocket);

        bool SharePeer(const Socket::Ptr& pTarget, const Socket::Ptr& pNewPeer);

    private:
        std::string m_address;
        uint16_t m_port = 0;

        const SocketType m_type = SocketType::Unknown;

        Context::Ptr m_context;

        Acceptor::Ptr m_acceptor;
        std::vector<Socket::Ptr> m_sockets;

        Callback m_onAccept;

    };
}


#endif //SR_UTILS_NETWORK_PEER_TO_PEER_H
