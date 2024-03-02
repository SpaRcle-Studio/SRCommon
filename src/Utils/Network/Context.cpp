//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Context.h>
#include <Utils/Network/Acceptor.h>
#include <Utils/Network/Socket.h>

#include <Utils/Network/Asio/AsioContext.h>

namespace SR_NETWORK_NS {
    Context::Context()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    SR_HTYPES_NS::SharedPtr<Context> Context::Create() {
        return AsioContext::MakeShared<AsioContext, Context>();
    }

    SR_HTYPES_NS::SharedPtr<Context> Context::CreateAndRun() {
        auto&& pContext = Create();
        if (!pContext->Run()) {
            SR_ERROR("Context::CreateAndRun() : failed to run context!");
            return nullptr;
        }

        return pContext;
    }

    SR_HTYPES_NS::SharedPtr<PeerToPeer> Context::CreateP2P(SocketType type, const std::string& address, uint16_t port) {
        return new PeerToPeer(type, GetThis(), address, port);
    }

    void Context::AddAsyncAcceptor(const SR_HTYPES_NS::SharedPtr<Acceptor>& pAcceptor) {
        m_asyncAcceptors.emplace_back(pAcceptor);
    }

    void Context::AddAsyncReceiveSocket(const SR_HTYPES_NS::SharedPtr<Socket>& pSocket) {
        m_asyncReceiveSockets.emplace_back(pSocket);
    }

    bool Context::Poll() {
        if (!m_isRunning) {
            SR_ERROR("Context::Pool() : context is not running!");
            return false;
        }

        uint32_t asyncAcceptorsCount = m_asyncAcceptors.size();
        while (asyncAcceptorsCount--) {
            auto pAcceptor = m_asyncAcceptors.front();
            m_asyncAcceptors.pop_front();

            if (!pAcceptor || !pAcceptor->IsOpen()) {
                continue;
            }

            if (!pAcceptor->StartInternal(true)) {
                SR_ERROR("Context::Poll() : failed to start acceptor!");
            }
        }

        uint32_t asyncReceiveSocketsCount = m_asyncReceiveSockets.size();
        while (asyncReceiveSocketsCount--) {
            auto pSocket = m_asyncReceiveSockets.front();
            m_asyncReceiveSockets.pop_front();

            if (!pSocket || !pSocket->IsOpen()) {
                continue;
            }

            if (!pSocket->ReceiveAsyncInternal()) {
                SR_ERROR("Context::Poll() : failed to start async receive!");
            }
        }

        uint32_t asyncSendKnownHostsSocketsCount = m_asyncSendKnownHostsSockets.size();
        while (asyncSendKnownHostsSocketsCount--) {
            const auto [pP2P, pSocket] = m_asyncSendKnownHostsSockets.front();
            m_asyncSendKnownHostsSockets.pop_front();

            if (!pP2P || !pSocket || !pSocket->IsOpen() || !pP2P->IsOpen()) {
                continue;
            }

            if (pP2P->GetNewPeersCount() > 0) {
                m_asyncSendKnownHostsSockets.emplace_back(pP2P, pSocket);
                continue;
            }

            if (!pP2P->SendKnownHosts(pSocket)) {
                SR_ERROR("Context::Poll() : failed to send known hosts!");
            }
        }

        return true;
    }

    void Context::AddAsyncSendKnownHostsSocket(const Context::PeerToPeerPtr& pP2P, const Context::SocketPtr& pSocket) {
        m_asyncSendKnownHostsSockets.emplace_back( pP2P, pSocket );
    }
}
