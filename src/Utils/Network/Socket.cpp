//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/Socket.h>
#include <Utils/Network/Context.h>
#include <Utils/Network/Acceptor.h>

namespace SR_NETWORK_NS {
    Socket::Socket(SocketType type, Context::Ptr context)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_type(type)
        , m_context(std::move(context))
    { }

    DataPackage::Ptr Socket::Receive(uint64_t size) {
        if (size == 0) {
            SR_ERROR("Socket::Receive() : invalid size!");
            return nullptr;
        }

        m_receivedData = DataPackage::Allocate(size);
        if (!Receive(m_receivedData->GetData(), size)) {
            return nullptr;
        }

        return std::move(m_receivedData);
    }

    bool Socket::AsyncReceive(uint64_t size) {
        if (m_isWaitingReceive) {
            SR_ERROR("Socket::AsyncReceive() : already waiting for async receive!");
            return false;
        }

        m_receivedAsyncData = DataPackage::Allocate(size);
        m_context->AddAsyncReceiveSocket(GetThis());

        return true;
    }
}