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
}
