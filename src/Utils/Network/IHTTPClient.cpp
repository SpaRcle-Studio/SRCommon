//
// Created by Monika on 07.09.2026.
//

#include <Utils/Network/IHTTPClient.h>

namespace SR_NETWORK_NS {
#if defined(SR_EMSCRIPTEN) || defined(SR_ANDROID) || defined(SR_LINUX)
    IHTTPClient* CreateHTTPClient() {
        SRHalt("IHTTPClient::Create() : not implemented!");
        return nullptr;
    }
#endif
}