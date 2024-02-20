//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/PeerToPeer.h>

namespace SR_NETWORK_NS {
    PeerToPeer::PeerToPeer(SocketType type, Context::Ptr pContext, std::string address, uint16_t port)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_address(std::move(address))
        , m_port(port)
        , m_type(type)
        , m_context(std::move(pContext))
    { }

    void PeerToPeer::OnAccept(Socket::Ptr&& pSocket) {
        m_sockets.push_back(std::move(pSocket));
        if (m_onAccept) {
            m_onAccept(GetThis(), m_sockets.back());
        }
    }

    bool PeerToPeer::Run() {
        m_acceptor = m_context->CreateAcceptor(m_type, m_address, m_port);
        if (!m_acceptor->StartAsync([this](auto&& pSocket) {
            OnAccept(std::forward<decltype(pSocket)>(pSocket));
        })) {
            SR_ERROR("PeerToPeer::Run() : failed to start acceptor!");
            return false;
        }

        return true;
    }
}