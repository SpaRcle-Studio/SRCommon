//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/PeerToPeer.h>
#include <Utils/Network/Utils.h>

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

        m_acceptor->SetCallback([this](auto&& pSocket) {
            OnAccept(std::forward<decltype(pSocket)>(pSocket));
        });

        if (!m_acceptor->StartAsync()) {
            SR_ERROR("PeerToPeer::Run() : failed to start acceptor!");
            return false;
        }

        return true;
    }

    void PeerToPeer::Close() {
        if (m_acceptor) {
            m_acceptor->Close();
        }

        for (auto&& pSocket : m_sockets) {
            pSocket->Close();
        }

        m_sockets.clear();
    }

    bool PeerToPeer::Connect(const std::string& address, uint16_t port) {
        auto pSocket = m_context->CreateSocket(m_type);
        if (!pSocket->Connect(address, port)) {
            SR_ERROR("PeerToPeer::Connect() : failed to connect to {}:{}!", address, port);
            return false;
        }

        for (auto pIt = m_sockets.begin(); pIt != m_sockets.end(); ) {
            auto pPeer = *pIt;

            if (pPeer->IsOpen()) {
                if (!SharePeer(pPeer, pSocket)) {
                    SR_ERROR("PeerToPeer::Connect() : failed to share peer!");
                }
                ++pIt;
            }
            else {
                pIt = m_sockets.erase(pIt);
            }
        }

        m_sockets.push_back(std::move(pSocket));

        return true;
    }

    bool PeerToPeer::SharePeer(const Socket::Ptr& pTarget, const Socket::Ptr& pNewPeer) {
        P2PConnectionHeader header;
        header.port = pNewPeer->GetRemotePort();
        header.iPv4 = SR_NETWORK_NS::StringToIPv4(pNewPeer->GetRemoteAddress());

        if (!pTarget->Send(&header, sizeof(header))) {
            SR_ERROR("PeerToPeer::SharePeer() : failed to send P2PConnectionHeader!");
            return false;
        }

        return true;
    }
}