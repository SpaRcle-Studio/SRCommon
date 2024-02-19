//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_NETWORK_PEER_TO_PEER_H
#define SR_UTILS_NETWORK_PEER_TO_PEER_H

#include <Utils/Network/Server.h>

namespace SR_NETWORK_NS {
    class PeerToPeer : public SR_HTYPES_NS::SharedPtr<PeerToPeer> {
    public:
        PeerToPeer(SocketType type, int32_t domain, int32_t service, int32_t protocol, int32_t port, uint64_t interface);

    private:


    };
}


#endif //SR_UTILS_NETWORK_PEER_TO_PEER_H
