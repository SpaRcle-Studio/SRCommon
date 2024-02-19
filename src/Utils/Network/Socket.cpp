//
// Created by Monika on 19.02.2024.
//

#include <Utils/Network/Socket.h>
#include <Utils/Network/AsioTCPSocket.h>

namespace SR_NETWORK_NS {
    Socket::Socket(SocketType type)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_type(type)
    { }

    SR_HTYPES_NS::SharedPtr<Socket> Socket::Create(SocketType type) {
        switch (type) {
            case SocketType::TCP:
                return AsioTCPSocket::MakeShared<AsioTCPSocket, Socket>();
            case SocketType::UDP:
                //return SR_HTYPES_NS::MakeShared<AsioUDPSocket>();
            default:
                SR_ERROR("Socket::Create() : unknown socket type: {}", SR_UTILS_NS::EnumReflector::ToStringAtom(type).c_str());
                return nullptr;
        }
    }
}