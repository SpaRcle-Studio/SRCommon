//
// Created by innerviewer on 2026-03-17.
//

#include <Utils/Network/GNS/GNSContext.h>
#include <Utils/Network/GNS/GNSSocket.h>
#include <Utils/Network/GNS/GNSAcceptor.h>
#include <Utils/Network/Pinger.h>
#include <Utils/Debug.h>

#ifdef SR_COMMON_GNS

namespace SR_NETWORK_NS {
    GNSContext* GNSContext::s_pCallbackInstance = nullptr;

    void GNSContext::GNSDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {
        switch (eType) {
            case k_ESteamNetworkingSocketsDebugOutputType_Bug:
            case k_ESteamNetworkingSocketsDebugOutputType_Error:
                SR_ERROR("GNS: {}", pszMsg);
                break;
            case k_ESteamNetworkingSocketsDebugOutputType_Important:
            case k_ESteamNetworkingSocketsDebugOutputType_Warning:
                SR_WARN("GNS: {}", pszMsg);
                break;
            default:
                SR_LOG("GNS: {}", pszMsg);
                break;
        }
    }

    GNSContext::~GNSContext() {
        if (m_isRunning) {
            SR_WARN("GNSContext::~GNSContext() : context is not stopped!");
            Stop();
        }
    }

    bool GNSContext::Run() {
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
            SR_ERROR("GNSContext::Run() : failed to initialize GameNetworkingSockets: {}", errMsg);
            return false;
        }

        /// Set up debug output to capture GNS internal messages.
        SteamNetworkingUtils()->SetDebugOutputFunction(
            k_ESteamNetworkingSocketsDebugOutputType_Msg,
            GNSDebugOutput
        );

        m_pInterface = SteamNetworkingSockets();
        if (!m_pInterface) {
            SR_ERROR("GNSContext::Run() : failed to get ISteamNetworkingSockets interface!");
            GameNetworkingSockets_Kill();
            return false;
        }

        s_pCallbackInstance = this;

        m_isRunning = true;

        SR_LOG("GNSContext::Run() : GameNetworkingSockets initialized successfully.");

        return true;
    }

    bool GNSContext::Poll() {
        if (!Super::Poll()) {
            SR_ERROR("GNSContext::Poll() : failed to poll super!");
            return false;
        }

        if (!m_pInterface) {
            SR_ERROR("GNSContext::Poll() : interface is null!");
            return false;
        }

        m_pInterface->RunCallbacks();

        return true;
    }

    void GNSContext::Stop() {
        if (!m_isRunning) {
            SR_ERROR("GNSContext::Stop() : context is not running!");
            return;
        }

        /// Close all tracked sockets and acceptors.
        for (auto&& [hConn, pSocket] : m_socketMap) {
            if (m_pInterface && hConn != k_HSteamNetConnection_Invalid) {
                m_pInterface->CloseConnection(hConn, 0, "Context shutdown", false);
            }
        }
        m_socketMap.clear();

        for (auto&& [hListenSocket, pAcceptor] : m_acceptorMap) {
            if (m_pInterface && hListenSocket != k_HSteamListenSocket_Invalid) {
                m_pInterface->CloseListenSocket(hListenSocket);
            }
        }
        m_acceptorMap.clear();

        if (s_pCallbackInstance == this) {
            s_pCallbackInstance = nullptr;
        }

        m_pInterface = nullptr;

        GameNetworkingSockets_Kill();

        m_isRunning = false;

        SR_LOG("GNSContext::Stop() : GameNetworkingSockets shut down.");
    }

    SR_HTYPES_NS::SharedPtr<Socket> GNSContext::CreateSocket(SocketType type) {
        if (type != SocketType::TCP && type != SocketType::UDP) {
            SR_ERROR("GNSContext::CreateSocket() : GNS only supports TCP-like (reliable) and UDP-like (unreliable) modes. "
                     "Use SocketType::TCP for reliable, SocketType::UDP for unreliable.");
            return nullptr;
        }

        auto* pSocket = new GNSSocket(GetThis());

        if (type == SocketType::UDP) {
            pSocket->SetSendMode(GNSSocket::SendMode::Unreliable);
        }

        return pSocket;
    }

    SR_HTYPES_NS::SharedPtr<Pinger> GNSContext::CreatePinger() {
        SR_ERROR("GNSContext::CreatePinger() : pinger is not supported with GameNetworkingSockets. "
                 "Use the Asio backend for ICMP ping functionality.");
        return nullptr;
    }

    SR_HTYPES_NS::SharedPtr<Acceptor> GNSContext::CreateAcceptor(SocketType type, const std::string& address, uint16_t port) {
        if (type != SocketType::TCP && type != SocketType::UDP) {
            SR_ERROR("GNSContext::CreateAcceptor() : GNS only supports TCP and UDP socket types.");
            return nullptr;
        }

        auto* pAcceptor = new GNSAcceptor(GetThis(), address, port);
        return pAcceptor;
    }

    void GNSContext::RegisterAcceptor(HSteamListenSocket hListenSocket, GNSAcceptor* pAcceptor) {
        m_acceptorMap[hListenSocket] = pAcceptor;
    }

    void GNSContext::UnregisterAcceptor(HSteamListenSocket hListenSocket) {
        m_acceptorMap.erase(hListenSocket);
    }

    void GNSContext::RegisterSocket(HSteamNetConnection hConn, GNSSocket* pSocket) {
        m_socketMap[hConn] = pSocket;
    }

    void GNSContext::UnregisterSocket(HSteamNetConnection hConn) {
        m_socketMap.erase(hConn);
    }

    void GNSContext::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
        if (s_pCallbackInstance) {
            s_pCallbackInstance->HandleConnectionStatusChanged(pInfo);
        }
    }

    void GNSContext::HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
        if (!pInfo) {
            return;
        }

        /// If the connection is on a listen socket, route to the appropriate acceptor.
        if (pInfo->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid) {
            auto&& it = m_acceptorMap.find(pInfo->m_info.m_hListenSocket);
            if (it != m_acceptorMap.end() && it->second) {
                if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting) {
                    it->second->OnIncomingConnection(pInfo->m_hConn);
                    return;
                }
            }
        }

        /// Route to the socket if we have one tracked.
        auto&& it = m_socketMap.find(pInfo->m_hConn);
        if (it != m_socketMap.end() && it->second) {
            it->second->OnConnectionStatusChanged(pInfo);
        }
    }
}

#endif