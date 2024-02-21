//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Context.h>

#include <Utils/Network/Asio/AsioContext.h>

namespace SR_NETWORK_NS {
    Context::Context()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    SR_HTYPES_NS::SharedPtr<Context> Context::Create() {
        return AsioContext::MakeShared<AsioContext, Context>();
    }

    SR_HTYPES_NS::SharedPtr<PeerToPeer> Context::CreateP2P(SocketType type, const std::string& address, uint16_t port) {
        return new PeerToPeer(type, GetThis(), address, port);
    }

    void Context::AddAsyncAcceptor(const SR_HTYPES_NS::SharedPtr<Acceptor>& pAcceptor) {
        m_asyncAcceptors.push_back(pAcceptor);
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

        return true;
    }
}
