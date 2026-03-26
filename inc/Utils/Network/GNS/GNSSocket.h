//
// Created by innerviewer on 2026-03-17.
//

#ifndef SR_UTILS_NETWORK_GNS_SOCKET_H
#define SR_UTILS_NETWORK_GNS_SOCKET_H

#include <Utils/Network/Socket.h>

#ifdef SR_COMMON_GNS
#include <steam/steamnetworkingsockets.h>

namespace SR_NETWORK_NS {
    class GNSContext;

    class GNSSocket : public Socket {
        using Super = Socket;
        friend class GNSContext;
        friend class GNSAcceptor;
    public:
        enum class SendMode : uint8_t {
            Reliable,
            Unreliable,
            NoNagle,
            UnreliableNoNagle
        };

    private:
        explicit GNSSocket(Context::Ptr pContext);

    public:
        ~GNSSocket() override;

    public:
        bool Connect(const std::string& address, uint16_t port) override;
        bool Send(const void* data, size_t size) override;
        bool SendTo(const void* data, uint64_t size, const std::string& address, uint16_t port) override;
        bool Close() override;

        uint64_t Receive(void* data, size_t size) override;
        uint64_t AsyncReceive(void* data, std::function<void(uint64_t bytesReceived)> callback) override;

        SR_NODISCARD bool IsOpen() const override;

        SR_NODISCARD std::string GetLocalAddress() const override;
        SR_NODISCARD std::string GetRemoteAddress() const override;

        SR_NODISCARD uint16_t GetLocalPort() const override;
        SR_NODISCARD uint16_t GetRemotePort() const override;

        void SetConnection(HSteamNetConnection hConn);
        void SetSendMode(SendMode mode) { m_sendMode = mode; }

        SR_NODISCARD HSteamNetConnection GetConnection() const { return m_hConn; }
        SR_NODISCARD SendMode GetSendMode() const { return m_sendMode; }

        /// Send with an explicit reliability mode.
        bool SendWithFlags(const void* data, size_t size, int32_t sendFlags);

        void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

    protected:
        bool ReceiveAsyncInternal() override;

    private:
        SR_NODISCARD int32_t GetSendFlags() const;

    private:
        HSteamNetConnection m_hConn = k_HSteamNetConnection_Invalid;
        SendMode m_sendMode = SendMode::Reliable;
    };
}
#endif

#endif //SR_UTILS_NETWORK_GNS_SOCKET_H
