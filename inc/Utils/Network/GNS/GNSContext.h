//
// Created by innerviewer on 2026-03-17.
//

#ifndef SR_UTILS_NETWORK_GNS_CONTEXT_H
#define SR_UTILS_NETWORK_GNS_CONTEXT_H

#include <Utils/Network/Context.h>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace SR_NETWORK_NS {
    class GNSAcceptor;
    class GNSSocket;

    class GNSContext : public Context {
        using Super = Context;
    public:
        GNSContext() = default;
        ~GNSContext() override;

    public:
        ISteamNetworkingSockets* GetInterface() const { return m_pInterface; }

        bool Run() override;
        bool Poll() override;
        void Stop() override;

        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Socket> CreateSocket(SocketType type) override;
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Pinger> CreatePinger() override;
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Acceptor> CreateAcceptor(SocketType type, const std::string& address, uint16_t port) override;

        void RegisterAcceptor(HSteamListenSocket hListenSocket, GNSAcceptor* pAcceptor);
        void UnregisterAcceptor(HSteamListenSocket hListenSocket);

        void RegisterSocket(HSteamNetConnection hConn, GNSSocket* pSocket);
        void UnregisterSocket(HSteamNetConnection hConn);

        static void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

    private:
        void HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
        static void GNSDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);

    private:
        ISteamNetworkingSockets* m_pInterface = nullptr;

        std::map<HSteamListenSocket, GNSAcceptor*> m_acceptorMap;
        std::map<HSteamNetConnection, GNSSocket*> m_socketMap;

        static GNSContext* s_pCallbackInstance;
    };
}

#endif //SR_UTILS_NETWORK_GNS_CONTEXT_H
