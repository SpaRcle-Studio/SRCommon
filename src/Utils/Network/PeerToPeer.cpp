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
        if (!ListerPeer(pSocket)) {
            SR_ERROR("PeerToPeer::OnAccept() : failed to lister peer!");
            return;
        }

        P2PAnnounceHeader header;
        header.iPv4 = SR_NETWORK_NS::StringToIPv4(m_acceptor->GetRemoteAddress());
        header.port = m_acceptor->GetRemotePort();

        if (!pSocket->Send(&header, sizeof(header))) {
            SR_ERROR("PeerToPeer::Connect() : failed to send P2PAnnounceHeader!");
            return;
        }

        m_newPeers.insert(pSocket);

        if (m_onAccept) {
            m_onAccept(GetThis(), pSocket);
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

        for (auto&& [pSocket, connection] : m_connections) {
            pSocket->Close();
        }

        m_connections.clear();

        if (!m_newPeers.empty()) {
            SR_WARN("PeerToPeer::Close() : new peers are not empty! Count {}", m_newPeers.size());
        }

        for (auto&& pSocket : m_newPeers) {
            pSocket->Close();
        }

        m_newPeers.clear();
    }

    bool PeerToPeer::Connect(const std::string& address, uint16_t port) {
        return ConnectInternal(address, port, true);
    }

    bool PeerToPeer::ConnectInternal(const std::string& address, uint16_t port, bool share) {
        if (!m_acceptor) {
            SR_ERROR("PeerToPeer::Connect() : acceptor is not running!");
            return false;
        }

        auto&& pSocket = m_context->CreateSocket(m_type);
        if (!pSocket->Connect(address, port)) {
            SR_ERROR("PeerToPeer::Connect() : failed to connect to {}:{}!", address, port);
            return false;
        }

        if (share) {
            P2PAnnounceHeader header;
            header.iPv4 = SR_NETWORK_NS::StringToIPv4(m_acceptor->GetRemoteAddress());
            header.port = m_acceptor->GetRemotePort();

            if (!pSocket->Send(&header, sizeof(header))) {
                SR_ERROR("PeerToPeer::Connect() : failed to send P2PAnnounceHeader!");
                return false;
            }
        }

        if (!ListerPeer(pSocket)) {
            SR_ERROR("PeerToPeer::Connect() : failed to lister peer!");
            return false;
        }

        m_newPeers.insert(pSocket);

        return true;
    }

    bool PeerToPeer::SharePeer(const Socket::Ptr& pTarget, const Socket::Ptr& pNewPeer) {
        P2PConnectionHeader header;
        header.port = m_connections[pNewPeer].port;
        header.iPv4 = m_connections[pNewPeer].address;

        //SR_LOG("PeerToPeer::SharePeer() : sharing peer {}:{} to {}:{}!",
        //       SR_NETWORK_NS::IPv4ToString(header.iPv4), header.port,
        //       m_connections[pTarget].address, m_connections[pTarget].port
        //);

        if (!pTarget->Send(&header, sizeof(header))) {
            SR_ERROR("PeerToPeer::SharePeer() : failed to send P2PConnectionHeader!");
            return false;
        }

        return true;
    }

    bool PeerToPeer::ListerPeer(const Socket::Ptr& pSocket) {
        pSocket->SetReceiveCallback([this](const Socket::Ptr& pSocket, const DataPackage::Ptr& pData, uint64_t size) {
            if (size < sizeof(P2PBaseHeader)) {
                SR_ERROR("PeerToPeer::ListerPeer() : invalid data size!");
                return;
            }
            ProcessMessage(pSocket, pData);
        });

        return pSocket->AsyncReceive(GetMaxP2PHeaderSize());
    }

    bool PeerToPeer::RegisterSocket(Socket::Ptr pSocket, uint32_t address, uint16_t port) {
        if (SR_NETWORK_NS::StringToIPv4(m_acceptor->GetRemoteAddress()) == address && m_acceptor->GetRemotePort() == port) {
            SR_ERROR("PeerToPeer::RegisterSocket() : {}:{} peer is the same as acceptor!",
                 SR_NETWORK_NS::IPv4ToString(address), port
            );
            pSocket->Close();
            return false;
        }

        if (HasConnection(address, port)) {
            SR_ERROR("PeerToPeer::RegisterSocket() : {}:{} peer is already connected!", SR_NETWORK_NS::IPv4ToString(address), port);
            pSocket->Close();
            return false;
        }

        if (!m_newPeers.count(pSocket)) {
            SR_ERROR("PeerToPeer::RegisterSocket() : {}:{} peer is not new!",
                 SR_NETWORK_NS::IPv4ToString(address), port
            );
            pSocket->Close();
            return false;
        }

        SR_LOG("PeerToPeer::RegisterSocket() : {}:{} registered in {}:{}!",
            SR_NETWORK_NS::IPv4ToString(address), port,
            m_acceptor->GetRemoteAddress(), m_acceptor->GetRemotePort()
        );

        m_newPeers.erase(pSocket);
        m_connections[std::move(pSocket)] = Connection(address, port);

        return true;
    }

    bool PeerToPeer::SharePeer(const Socket::Ptr& pNewPeer) {
        for (auto pIt = m_connections.begin(); pIt != m_connections.end(); ) {
            auto pPeer = pIt->first;

            if (pNewPeer == pPeer) {
                ++pIt;
                continue;
            }

            if (pPeer->IsOpen()) {
                if (!SharePeer(pPeer, pNewPeer)) {
                    SR_ERROR("PeerToPeer::Connect() : failed to share peer!");
                }
                ++pIt;
            }
            else {
                pIt = m_connections.erase(pIt);
            }
        }

        return true;
    }

    bool PeerToPeer::SendKnownHosts(const Socket::Ptr& pSocket) {
        SRAssert2(!m_connections.empty(), "PeerToPeer::SendKnownHosts() : connections are empty!");

        if (!m_newPeers.empty()) {
            SR_WARN("PeerToPeer::SendKnownHosts() : new peers are not empty! Count {}", m_newPeers.size());
        }

        for (auto&& [pPeer, connection] : m_connections) {
            if (pPeer == pSocket) {
                continue;
            }

            P2PKnownHostHeader header;
            header.iPv4 = connection.address;
            header.port = connection.port;

            if (!pSocket->Send(&header, sizeof(header))) {
                SR_ERROR("PeerToPeer::SendKnownHosts() : failed to send P2PConnectionHeader!");
                return false;
            }
        }

        return true;
    }

    void PeerToPeer::ProcessMessage(const Socket::Ptr& pSocket, const DataPackage::Ptr& pData) {
        auto&& pBaseHeader = static_cast<const P2PConnectionHeader*>(pData->GetData());

        const bool isRegistered = m_connections.find(pSocket) != m_connections.end();

        if (!isRegistered) {
            if (pBaseHeader->type == P2PMessageType::Announce) {
                auto&& pHeader = static_cast<const P2PAnnounceHeader*>(pData->GetData());
                if (pHeader->iPv4 == 0 || pHeader->port == 0) {
                    SR_ERROR("PeerToPeer::ProcessMessage() : invalid P2PHostAddressHeader!");
                    return;
                }

                if (HasConnection(pHeader->iPv4, pHeader->port)) {
                    m_context->AddAsyncSendKnownHostsSocket(GetThis(), pSocket);
                    m_newPeers.erase(pSocket);
                    return;
                }

                if (!RegisterSocket(pSocket, pHeader->iPv4, pHeader->port)) {
                    SR_LOG("PeerToPeer::ProcessMessage() : {}:{} socket already connected!",
                           m_acceptor->GetRemoteAddress(), m_acceptor->GetRemotePort()
                    );
                    m_newPeers.erase(pSocket);
                }

                m_context->AddAsyncSendKnownHostsSocket(GetThis(), pSocket);
            }
            return;
        }

        if (pBaseHeader->type == P2PMessageType::KnownHost) {
            auto&& pHeader = static_cast<const P2PKnownHostHeader*>(pData->GetData());
            if (pHeader->iPv4 == 0 || pHeader->port == 0) {
                SR_ERROR("PeerToPeer::ProcessMessage() : invalid P2PKnownHostHeader!");
                return;
            }

            if (!Connect(SR_NETWORK_NS::IPv4ToString(pHeader->iPv4), pHeader->port)) {
                SR_ERROR("PeerToPeer::ProcessMessage() : failed to connect to peer!");
            }
        }

        /*switch (pBaseHeader->type) {
            case P2PMessageType::KnownHostsListInfo: {
                auto&& pListInfoHeader = static_cast<const P2PKnownHostsListInfoHeader*>(pData->GetData());
                SR_LOG("PeerToPeer::ProcessMessage() : received known hosts list info: {}!", pListInfoHeader->count);
                break;
            }
            case P2PMessageType::HostAddress: {
                auto&& pHeader = static_cast<const P2PHostAddressHeader*>(pData->GetData());
                if (pHeader->iPv4 == 0 || pHeader->port == 0) {
                    SR_ERROR("PeerToPeer::ProcessMessage() : invalid P2PHostAddressHeader!");
                    return;
                }

                SR_LOG("PeerToPeer::ProcessMessage() : {}:{} received host address: {}:{}!",
                       m_acceptor->GetRemoteAddress(), m_acceptor->GetRemotePort(),
                       SR_NETWORK_NS::IPv4ToString(pHeader->iPv4), pHeader->port
                );

                if (pHeader->share) {
                    m_connections[pSocket].address = pHeader->iPv4;
                    m_connections[pSocket].port = pHeader->port;

                    if (!SharePeer(pSocket)) {
                        SR_ERROR("PeerToPeer::ProcessMessage() : failed to share peer!");
                    }
                }

                break;
            };
            case P2PMessageType::PeerConnection: {
                auto&& pHeader = static_cast<const P2PConnectionHeader*>(pData->GetData());
                if (pHeader->iPv4 == 0 || pHeader->port == 0) {
                    SR_ERROR("PeerToPeer::ProcessMessage() : invalid P2PConnectionHeader!");
                    return;
                }

                SR_LOG("PeerToPeer::ProcessMessage() : {}:{} received peer connection: {}:{}!",
                       m_acceptor->GetRemoteAddress(), m_acceptor->GetRemotePort(),
                       SR_NETWORK_NS::IPv4ToString(pHeader->iPv4), pHeader->port
                );

                //if (!ConnectInternal(SR_NETWORK_NS::IPv4ToString(pHeader->iPv4), pHeader->port, false)) {
                //    SR_ERROR("PeerToPeer::ProcessMessage() : failed to connect to peer!");
                //}

                break;
            }
            default:
                SR_ERROR("PeerToPeer::ProcessMessage() : invalid P2PMessageType!");
                break;
        }*/
    }

    bool PeerToPeer::HasConnection(uint32_t address, uint16_t port) const {
        for (auto&& [pPeer, info] : m_connections) {
            if (info.address == address && info.port == port) {
                return true;
            }
        }
        return false;
    }
}