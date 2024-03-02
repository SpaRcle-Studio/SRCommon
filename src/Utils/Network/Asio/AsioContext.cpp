//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Asio/AsioContext.h>
#include <Utils/Network/Asio/AsioTCPSocket.h>
#include <Utils/Network/Asio/AsioICMPSocket.h>
#include <Utils/Network/Asio/AsioTCPAcceptor.h>

namespace SR_NETWORK_NS {
    AsioContext::~AsioContext() {
        if (m_isRunning) {
            SR_WARN("AsioContext::~AsioContext() : context is not stopped!");
            m_context.stop();
        }
    }

    SR_HTYPES_NS::SharedPtr<Socket> AsioContext::CreateSocket(SocketType type) {
        switch (type) {
            case SocketType::TCP:
                return new AsioTCPSocket(GetThis());
            case SocketType::UDP:
                // return new AsioUDPSocket(GetThis());
            case SocketType::ICMP:
                return new AsioICMPSocket(GetThis());
            default:
                SR_ERROR("AsioContext::CreateSocket() : unknown socket type: {}", SR_UTILS_NS::EnumReflector::ToStringAtom(type).c_str());
                return nullptr;
        }
    }

    SR_HTYPES_NS::SharedPtr<Acceptor> AsioContext::CreateAcceptor(SocketType type, const std::string& address, uint16_t port) {
        switch (type) {
            case SocketType::TCP: {
                auto&& pAcceptor = new AsioTCPAcceptor(GetThis(), address, port);
                return pAcceptor;
            }
            case SocketType::UDP:
                // return new AsioUDPAcceptor(GetThis(), address, port);
            default:
                SR_ERROR("AsioContext::CreateAcceptor() : unknown socket type: {}", SR_UTILS_NS::EnumReflector::ToStringAtom(type).c_str());
                return nullptr;
        }
    }

    bool AsioContext::Run() {
        asio::error_code errorCode;

        m_context.run(errorCode);

        if (errorCode) {
            SR_ERROR("AsioContext::Run() : failed to run context: {}", errorCode.message());
            return false;
        }

        m_isRunning = true;

        return true;
    }

    bool AsioContext::Poll() {
        asio::error_code errorCode;

        if (!Super::Poll()) {
            SR_ERROR("AsioContext::Pool() : failed to poll super!");
            return false;
        }

        m_context.poll(errorCode);

        if (errorCode) {
            SR_ERROR("AsioContext::Pool() : failed to pool context: {}", errorCode.message());
            return false;
        }

        return true;
    }

    void AsioContext::Stop() {
        if (!m_isRunning) {
            SR_ERROR("AsioContext::Stop() : context is not running!");
            return;
        }

        m_context.stop();
        m_isRunning = false;
    }
}