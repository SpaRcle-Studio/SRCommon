//
// Created by innerviewer on 2026-03-17.
//

#include <Utils/Network/GNS/GNSSocket.h>
#include <Utils/Network/GNS/GNSContext.h>
#include <Utils/Debug.h>

#ifdef SR_COMMON_GNS

namespace SR_NETWORK_NS {
    GNSSocket::GNSSocket(Context::Ptr pContext)
        : Super(SocketType::TCP, std::move(pContext))
    { }

    GNSSocket::~GNSSocket() {
        if (m_hConn != k_HSteamNetConnection_Invalid) {
            SR_WARN("GNSSocket::~GNSSocket() : connection is still open, closing it.");
            Close();
        }
    }

    bool GNSSocket::Connect(const std::string& address, uint16_t port) {
        if (IsOpen()) {
            SR_ERROR("GNSSocket::Connect() : connection is already open!");
            return false;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::Connect() : invalid GNS context or interface!");
            return false;
        }

        SteamNetworkingIPAddr serverAddr;
        serverAddr.Clear();

        if (!serverAddr.ParseString(std::string(address + ":" + std::to_string(port)).c_str())) {
            SR_ERROR("GNSSocket::Connect() : failed to parse address: {}:{}", address, port);
            return false;
        }

        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
            reinterpret_cast<void*>(&GNSContext::OnConnectionStatusChanged));

        m_hConn = pGNSContext->GetInterface()->ConnectByIPAddress(serverAddr, 1, &opt);

        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::Connect() : failed to connect to {}:{}!", address, port);
            return false;
        }

        pGNSContext->RegisterSocket(m_hConn, this);

        return true;
    }

    bool GNSSocket::Send(const void* data, size_t size) {
        return SendWithFlags(data, size, GetSendFlags());
    }

    bool GNSSocket::SendWithFlags(const void* data, size_t size, int32_t sendFlags) {
        if (size == 0) {
            SR_ERROR("GNSSocket::Send() : invalid size!");
            return false;
        }

        if (!data) {
            SR_ERROR("GNSSocket::Send() : invalid data!");
            return false;
        }

        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::Send() : invalid connection handle!");
            return false;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::Send() : invalid GNS context!");
            return false;
        }

        const EResult result = pGNSContext->GetInterface()->SendMessageToConnection(
            m_hConn, data, static_cast<uint32_t>(size), sendFlags, nullptr
        );

        if (result != k_EResultOK) {
            SR_ERROR("GNSSocket::Send() : failed to send data, result: {}", static_cast<int32_t>(result));
            return false;
        }

        return true;
    }

    bool GNSSocket::SendTo(const void* data, uint64_t size, const std::string& address, uint16_t port) {
        /// GNS is connection-oriented; SendTo is not applicable.
        /// Data is always sent to the connected peer.
        SR_ERROR("GNSSocket::SendTo() : not supported with GNS. Use Send() on a connected socket instead.");
        return false;
    }

    uint64_t GNSSocket::Receive(void* data, size_t size) {
        if (size == 0) {
            SR_ERROR("GNSSocket::Receive() : invalid size!");
            return 0;
        }

        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::Receive() : invalid connection handle!");
            return 0;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::Receive() : invalid GNS context!");
            return 0;
        }

        SteamNetworkingMessage_t* pIncomingMsg = nullptr;
        const int numMsgs = pGNSContext->GetInterface()->ReceiveMessagesOnConnection(m_hConn, &pIncomingMsg, 1);

        if (numMsgs <= 0 || !pIncomingMsg) {
            return 0;
        }

        const uint64_t copySize = SR_MIN(static_cast<uint64_t>(pIncomingMsg->m_cbSize), static_cast<uint64_t>(size));
        memcpy(data, pIncomingMsg->m_pData, copySize);

        pIncomingMsg->Release();

        return copySize;
    }

    uint64_t GNSSocket::AsyncReceive(void* data, std::function<void(uint64_t bytesReceived)> callback) {
        SRHalt("GNSSocket::AsyncReceive() : use the polling-based async receive instead!");
        return 0;
    }

    bool GNSSocket::Close() {
        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::Close() : connection is already closed!");
            return false;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (pGNSContext) {
            pGNSContext->UnregisterSocket(m_hConn);

            if (pGNSContext->GetInterface()) {
                pGNSContext->GetInterface()->CloseConnection(m_hConn, 0, "Connection closed", false);
            }
        }

        m_hConn = k_HSteamNetConnection_Invalid;

        return true;
    }

    bool GNSSocket::IsOpen() const {
        if (m_hConn == k_HSteamNetConnection_Invalid) {
            return false;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            return false;
        }

        SteamNetConnectionInfo_t info;
        if (!pGNSContext->GetInterface()->GetConnectionInfo(m_hConn, &info)) {
            return false;
        }

        return info.m_eState == k_ESteamNetworkingConnectionState_Connected
            || info.m_eState == k_ESteamNetworkingConnectionState_Connecting;
    }

    std::string GNSSocket::GetLocalAddress() const {
        /// GNS does not expose the local address through SteamNetConnectionInfo_t.
        /// Return "0.0.0.0" as a placeholder; the remote address is the meaningful one.
        return "0.0.0.0";
    }

    std::string GNSSocket::GetRemoteAddress() const {
        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::GetRemoteAddress() : invalid connection!");
            return std::string();
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::GetRemoteAddress() : invalid context!");
            return std::string();
        }

        SteamNetConnectionInfo_t info;
        if (!pGNSContext->GetInterface()->GetConnectionInfo(m_hConn, &info)) {
            SR_ERROR("GNSSocket::GetRemoteAddress() : failed to get connection info!");
            return std::string();
        }

        char buf[SteamNetworkingIPAddr::k_cchMaxString];
        info.m_addrRemote.ToString(buf, sizeof(buf), false);
        return std::string(buf);
    }

    uint16_t GNSSocket::GetLocalPort() const {
        /// GNS does not expose the local port through SteamNetConnectionInfo_t.
        /// Return 0 as a placeholder; the remote port is the meaningful one.
        return 0;
    }

    uint16_t GNSSocket::GetRemotePort() const {
        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::GetRemotePort() : invalid connection!");
            return 0;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::GetRemotePort() : invalid context!");
            return 0;
        }

        SteamNetConnectionInfo_t info;
        if (!pGNSContext->GetInterface()->GetConnectionInfo(m_hConn, &info)) {
            SR_ERROR("GNSSocket::GetRemotePort() : failed to get connection info!");
            return 0;
        }

        return info.m_addrRemote.m_port;
    }

    void GNSSocket::SetConnection(HSteamNetConnection hConn) {
        m_hConn = hConn;

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (pGNSContext && hConn != k_HSteamNetConnection_Invalid) {
            pGNSContext->RegisterSocket(hConn, this);
        }
    }

    bool GNSSocket::ReceiveAsyncInternal() {
        if (m_hConn == k_HSteamNetConnection_Invalid) {
            SR_ERROR("GNSSocket::ReceiveAsyncInternal() : invalid connection handle!");
            return false;
        }

        auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
        if (!pGNSContext || !pGNSContext->GetInterface()) {
            SR_ERROR("GNSSocket::ReceiveAsyncInternal() : invalid GNS context!");
            return false;
        }

        SteamNetworkingMessage_t* pIncomingMsg = nullptr;
        const int numMsgs = pGNSContext->GetInterface()->ReceiveMessagesOnConnection(m_hConn, &pIncomingMsg, 1);

        if (numMsgs <= 0 || !pIncomingMsg) {
            /// No messages available right now; re-register for the next poll cycle.
            if (m_isReceiveRepeated) {
                m_context->AddAsyncReceiveSocket(GetThis());
            }
            return true;
        }

        m_isWaitingReceive = false;

        const uint64_t msgSize = static_cast<uint64_t>(pIncomingMsg->m_cbSize);

        /// Copy received data into our async data buffer, expanding it if needed.
        if (!m_receivedAsyncData || m_receivedAsyncData->GetSize() < msgSize) {
            m_receivedAsyncData = DataPackage::Allocate(msgSize);
        }

        memcpy(m_receivedAsyncData->GetData(), pIncomingMsg->m_pData, msgSize);
        pIncomingMsg->Release();

        if (auto&& pReceiveCallback = GetReceiveCallback()) {
            pReceiveCallback(GetThis(), m_receivedAsyncData, msgSize);
        }

        if (m_isReceiveRepeated) {
            m_context->AddAsyncReceiveSocket(GetThis());
        }

        return true;
    }

    void GNSSocket::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
        if (!pInfo) {
            return;
        }

        switch (pInfo->m_info.m_eState) {
            case k_ESteamNetworkingConnectionState_Connected:
                SR_LOG("GNSSocket::OnConnectionStatusChanged() : connected to {}",
                    pInfo->m_info.m_szConnectionDescription);
                break;

            case k_ESteamNetworkingConnectionState_ClosedByPeer:
            case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                SR_LOG("GNSSocket::OnConnectionStatusChanged() : connection closed: {}",
                    pInfo->m_info.m_szEndDebug);

                {
                    auto&& pGNSContext = m_context.DynamicCast<GNSContext>();
                    if (pGNSContext && pGNSContext->GetInterface()) {
                        pGNSContext->GetInterface()->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                        pGNSContext->UnregisterSocket(pInfo->m_hConn);
                    }
                    m_hConn = k_HSteamNetConnection_Invalid;
                }
                break;

            default:
                break;
        }
    }

    int32_t GNSSocket::GetSendFlags() const {
        switch (m_sendMode) {
            case SendMode::Reliable:
                return k_nSteamNetworkingSend_Reliable;
            case SendMode::Unreliable:
                return k_nSteamNetworkingSend_Unreliable;
            case SendMode::NoNagle:
                return k_nSteamNetworkingSend_Reliable | k_nSteamNetworkingSend_NoNagle;
            case SendMode::UnreliableNoNagle:
                return k_nSteamNetworkingSend_Unreliable | k_nSteamNetworkingSend_NoNagle;
            default:
                return k_nSteamNetworkingSend_Reliable;
        }
    }
}

#endif