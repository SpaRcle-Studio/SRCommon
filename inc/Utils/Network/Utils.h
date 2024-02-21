//
// Created by Monika on 21.02.2024.
//

#ifndef SR_UTILS_NETWORK_UTILS_H
#define SR_UTILS_NETWORK_UTILS_H

#include <Utils/Common/Enumerations.h>

namespace SR_NETWORK_NS {
    SR_ENUM_NS_CLASS_T(P2PMessageType, uint8_t,
        Unknown,
        PeerConnection,
        PeerDisconnection
    )

    struct P2PBaseHeader {
        P2PMessageType type = P2PMessageType::Unknown;
    };

    struct P2PConnectionHeader {
        P2PMessageType type = P2PMessageType::PeerConnection;
        uint8_t version = 0;
        uint32_t iPv4 = 0;
        uint16_t port = 0;
    };

    struct P2PDisconnectionHeader {
        P2PMessageType type = P2PMessageType::PeerDisconnection;
        uint8_t version = 0;
        uint32_t iPv4 = 0;
        uint16_t port = 0;
    };

    /// write constexpr code that be return max header size from sizeof
    SR_MAYBE_UNUSED SR_CONSTEXPR SR_INLINE_STATIC size_t GetMaxP2PHeaderSize() {
        return SR_MAX(sizeof(P2PBaseHeader)
             , SR_MAX(sizeof(P2PConnectionHeader),
                      sizeof(P2PDisconnectionHeader)
        ));
    }

    SR_MAYBE_UNUSED SR_INLINE_STATIC uint32_t StringToIPv4(const std::string& address) {
        std::stringstream ss(address);
        uint32_t a, b, c, d;
        int8_t dot;
        ss >> a >> dot >> b >> dot >> c >> dot >> d;
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

    SR_MAYBE_UNUSED SR_INLINE_STATIC std::string IPv4ToString(uint32_t address) {
        std::stringstream ss;
        ss << ((address >> 24) & 0xFF) << '.' << ((address >> 16) & 0xFF) << '.' << ((address >> 8) & 0xFF) << '.' << (address & 0xFF);
        return ss.str();
    }
}

#endif //SR_UTILS_NETWORK_UTILS_H
