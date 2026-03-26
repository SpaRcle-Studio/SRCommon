//
// Created by innerviewer on 2026-03-17.
//

#ifndef SR_UTILS_NETWORK_GNS_ACCEPTOR_H
#define SR_UTILS_NETWORK_GNS_ACCEPTOR_H

#include <Utils/Network/Acceptor.h>

#include <steam/steamnetworkingsockets.h>

namespace SR_NETWORK_NS {
    class GNSContext;

    class GNSAcceptor : public Acceptor {
        using Super = Acceptor;
        friend class GNSContext;
    private:
        GNSAcceptor(Context::Ptr pContext, std::string address, uint16_t port);

    public:
        ~GNSAcceptor() override;

    public:
        void Close() override;

        SR_NODISCARD std::string GetLocalAddress() const override;
        SR_NODISCARD std::string GetRemoteAddress() const override;

        SR_NODISCARD uint16_t GetLocalPort() const override;
        SR_NODISCARD uint16_t GetRemotePort() const override;

        SR_NODISCARD HSteamListenSocket GetListenSocket() const { return m_hListenSocket; }

        /// Called by GNSContext when a new connection arrives on this listen socket.
        void OnIncomingConnection(HSteamNetConnection hConn);

    private:
        bool Init() override;
        bool StartInternal(bool async) override;

    private:
        HSteamListenSocket m_hListenSocket = k_HSteamListenSocket_Invalid;
    };
}

#endif //SR_UTILS_NETWORK_GNS_ACCEPTOR_H
