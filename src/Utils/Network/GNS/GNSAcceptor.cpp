//
// Created by innerviewer on 2026-03-17.
//

#include <Utils/Network/GNS/GNSAcceptor.h>
#include <Utils/Network/GNS/GNSContext.h>
#include <Utils/Network/GNS/GNSSocket.h>
#include <Utils/Debug.h>

#ifdef SR_COMMON_GNS

namespace SR_NETWORK_NS {
    GNSAcceptor::GNSAcceptor(Context::Ptr pContext, std::string address, uint16_t port)
        : Super(SocketType::TCP, std::move(pContext), std::move(address), port)
    { }

    GNSAcceptor::~GNSAcceptor() {
        if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
            SR_WARN("GNSAcceptor::~GNSAcceptor() : listen socket is still open, closing it.");
            Close();
        }
    }

    bool GNSAcceptor::Init() {
        if (!IsOpen()) {
            SR_ERROR("GNSAcceptor::Init() : acceptor is not open!");
            return false;
        }

        if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
            /// Already initialized.
            return true;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSAcceptor::Init() : invalid GNS context or interface!");
            return false;
        }

        SteamNetworkingIPAddr listenAddr;
        listenAddr.Clear();

        SR_LOG("GNSAcceptor::Init() : attempting to listen on address '{}' port {}", m_address, m_port);

        if (m_address.empty() || m_address == "0.0.0.0") {
            listenAddr.m_port = m_port;
        }
        else {
            const std::string addrStr = m_address + ":" + std::to_string(m_port);
            if (!listenAddr.ParseString(addrStr.c_str())) {
                SR_ERROR("GNSAcceptor::Init() : failed to parse listen address: {}", addrStr);
                return false;
            }
        }

        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
            reinterpret_cast<void*>(&GNSContext::OnConnectionStatusChanged));

        m_hListenSocket = pGNSContext->GetInterface()->CreateListenSocketIP(listenAddr, 1, &opt);

        if (m_hListenSocket == k_HSteamListenSocket_Invalid) {
            SR_ERROR("GNSAcceptor::Init() : failed to create listen socket on {}:{}!", m_address, m_port);
            return false;
        }

        pGNSContext->RegisterAcceptor(m_hListenSocket, this);

        SR_LOG("GNSAcceptor::Init() : listening on {}:{}", m_address, m_port);

        return true;
    }

    bool GNSAcceptor::StartInternal(bool async) {
        if (IsWaitingAccept()) {
            SRHalt("GNSAcceptor::StartInternal() : acceptor is already waiting for accept!");
            return false;
        }

        if (!Init()) {
            SR_ERROR("GNSAcceptor::StartInternal() : failed to init acceptor!");
            return false;
        }

        /// GNS acceptor is inherently async -- incoming connections are handled
        /// via the connection status callback in GNSContext. For synchronous mode,
        /// we would need to busy-wait, which is not practical.
        if (!async) {
            SR_WARN("GNSAcceptor::StartInternal() : synchronous accept is not supported with GNS. "
                    "Using async mode instead.");
        }

        m_isWaitingAccept = true;

        /// No explicit poll registration needed here; GNS handles it through
        /// RunCallbacks() in GNSContext::Poll(). Incoming connections arrive
        /// via OnIncomingConnection().

        return true;
    }

    void GNSAcceptor::Close() {
        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();

        if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
            if (pGNSContext) {
                pGNSContext->UnregisterAcceptor(m_hListenSocket);

                if (pGNSContext->GetInterface()) {
                    pGNSContext->GetInterface()->CloseListenSocket(m_hListenSocket);
                }
            }
            m_hListenSocket = k_HSteamListenSocket_Invalid;
        }

        Super::Close();
    }

    void GNSAcceptor::OnIncomingConnection(HSteamNetConnection hConn) {
        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSAcceptor::OnIncomingConnection() : invalid GNS context!");
            pGNSContext->GetInterface()->CloseConnection(hConn, 0, "Invalid context", false);
            return;
        }

        /// Accept the incoming connection.
        if (pGNSContext->GetInterface()->AcceptConnection(hConn) != k_EResultOK) {
            SR_ERROR("GNSAcceptor::OnIncomingConnection() : failed to accept connection!");
            pGNSContext->GetInterface()->CloseConnection(hConn, 0, "Failed to accept", false);
            return;
        }

        /// Create a GNSSocket wrapper for the accepted connection.
        auto&& pSocket = pGNSContext->CreateSocket(SocketType::TCP);
        pSocket.DynamicCast<GNSSocket>()->SetConnection(hConn);

        if (IsOpen() && IsRepeated()) {
            /// GNS continues listening automatically; no need to re-register.
            m_isWaitingAccept = true;
        }

        if (m_callback) {
            m_callback(std::move(pSocket));
        }
        else {
            SR_ERROR("GNSAcceptor::OnIncomingConnection() : callback is not set!");
            pSocket->Close();
        }
    }

    std::string GNSAcceptor::GetLocalAddress() const {
        return m_address;
    }

    std::string GNSAcceptor::GetRemoteAddress() const {
        /// For a listen socket, the "remote" address is the local address we're bound to.
        return m_address;
    }

    uint16_t GNSAcceptor::GetLocalPort() const {
        return m_port;
    }

    uint16_t GNSAcceptor::GetRemotePort() const {
        return m_port;
    }
}

#endif
