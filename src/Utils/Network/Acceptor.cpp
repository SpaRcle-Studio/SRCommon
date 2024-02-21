//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Acceptor.h>

namespace SR_NETWORK_NS {
    bool Acceptor::StartAsync() {
        if (!m_callback) {
            SR_ERROR("Acceptor::StartAsync() : callback is not set!");
            return false;
        }

        m_isOpen = true;

        if (!Init()) {
            SR_ERROR("Acceptor::StartAsync() : failed to init acceptor!");
            return false;
        }

        m_context->AddAsyncAcceptor(GetThis());
        return true;
    }

    bool Acceptor::Start()  {
        if (!m_callback) {
            SR_ERROR("Acceptor::StartAsync() : callback is not set!");
            return false;
        }

        m_isOpen = true;

        StartInternal(false);

        return true;
    }
}