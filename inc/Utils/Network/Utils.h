//
// Created by Monika on 21.02.2024.
//

#ifndef SR_UTILS_NETWORK_UTILS_H
#define SR_UTILS_NETWORK_UTILS_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/Function.h>

namespace SR_NETWORK_NS {
    SR_ENUM_NS_CLASS_T(P2PMessageType, uint8_t,
        Unknown,
        PeerConnection,
        PeerDisconnection,
        KnownHostsListInfo,
        KnownHost,
        Announce
    )

    struct P2PBaseHeader {
        P2PMessageType type = P2PMessageType::Unknown;
    };

    struct P2PAnnounceHeader {
        P2PMessageType type = P2PMessageType::Announce;
        uint8_t version = 0;
        uint32_t iPv4 = 0;
        uint16_t port = 0;
    };

    struct P2PKnownHostHeader {
        P2PMessageType type = P2PMessageType::KnownHost;
        uint8_t version = 0;
        uint32_t iPv4 = 0;
        uint16_t port = 0;
    };

    struct P2PKnownHostsListInfoHeader {
        P2PMessageType type = P2PMessageType::KnownHostsListInfo;
        uint8_t version = 0;
        uint64_t count = 0;
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

    class DataPackage : public SR_HTYPES_NS::SharedPtr<DataPackage> {
        using Super = SR_HTYPES_NS::SharedPtr<DataPackage>;
    public:
        DataPackage(void* pData, uint64_t size, bool copy)
            : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        {
            m_size = size;

            if (copy) {
                m_data = SRMalloc(size);
                memcpy(m_data, pData, size);
            }
            else {
                m_data = pData;
            }
        }

        virtual ~DataPackage() {
            if (m_data) {
                SRFree(m_data);
                m_data = nullptr;
            }
        }

        static SR_HTYPES_NS::SharedPtr<DataPackage> Allocate(uint64_t size) {
            auto&& pData = SRMalloc(size);
            return DataPackage::MakeShared(pData, size, false);
        }

        SR_NODISCARD const void* GetData() const { return m_data; }
        SR_NODISCARD void* GetData() { return m_data; }
        SR_NODISCARD uint64_t GetSize() const { return m_size; }

    private:
        void* m_data = nullptr;
        uint64_t m_size = 0;

    };

    /// write constexpr code that be return max header size from sizeof
    SR_MAYBE_UNUSED SR_CONSTEXPR SR_INLINE_STATIC size_t GetMaxP2PHeaderSize() {
        return
               SR_MAX(sizeof(P2PKnownHostHeader),
               SR_MAX(sizeof(P2PConnectionHeader),
               SR_MAX(sizeof(P2PDisconnectionHeader),
               SR_MAX(sizeof(P2PKnownHostsListInfoHeader),
               SR_MAX(sizeof(P2PAnnounceHeader),
                      sizeof(P2PBaseHeader)
               )))));
    }

    SR_MAYBE_UNUSED SR_INLINE_STATIC uint32_t StringToIPv4(StringView address) {
        uint32_t result = 0;
        uint32_t octet = 0;

        for (const char c : address) {
            if (c == '.') {
                result = (result << 8) | (octet & 0xFF);
                octet = 0;
            }
            else if (c >= '0' && c <= '9') {
                octet = octet * 10 + static_cast<uint32_t>(c - '0');
            }
            else {
                break;
            }
        }

        return (result << 8) | (octet & 0xFF);
    }

    SR_MAYBE_UNUSED SR_INLINE_STATIC String IPv4ToString(uint32_t address) {
        String result;
        result.reserve(15);

        for (int32_t shift = 24; shift >= 0; shift -= 8) {
            char buffer[4];
            auto&& [pEnd, errorCode] = std::to_chars(buffer, buffer + sizeof(buffer), (address >> shift) & 0xFF);
            result.append(buffer, static_cast<SizeType>(pEnd - buffer));

            if (shift > 0) {
                result.append('.');
            }
        }

        return result;
    }
}

#endif //SR_UTILS_NETWORK_UTILS_H
