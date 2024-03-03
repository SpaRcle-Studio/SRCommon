//
// Created by innerviewer on 2024-02-20.
//

#include <Utils/Network/Asio/AsioPinger.h>
#include <Utils/Network/Asio/AsioContext.h>
#include <Utils/Network/Headers/ICMPHeader.h>
#include <Utils/Network/Headers/IPV4Header.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/EnumReflector.h>

#include <asio.hpp>

namespace SR_NETWORK_NS {
    AsioPinger::~AsioPinger() {
        m_receiveTimer.Cancel();
        m_sendTimer.Cancel();
        m_socket->Close();
        m_context->Stop();
    }

    void AsioPinger::StartSend() {
        std::string body("\"Hello!\" from SpaRcle Network ping.");

        // Create an ICMP header for an echo request.
        ICMPHeader echoRequest;
        echoRequest.Type(ICMPType::EchoRequest);
        echoRequest.Code(static_cast<ICMPType>(0));
        echoRequest.Identifier(SR_PLATFORM_NS::GetCurrentProcessId());
        echoRequest.SequenceNumber(++m_sequenceNumber);
        ComputeChecksum(echoRequest, body.begin(), body.end());

        // Encode the request packet.
        asio::streambuf requestBuffer;
        std::ostream os(&requestBuffer);
        os << echoRequest << body;

        SR_LOG("AsioPinger::StartSend() : sending request.");
        m_sendTime = SR_UTILS_NS::TimePointType::clock::now();

        m_socket->SendTo(requestBuffer.data().data(), requestBuffer.size(), m_destination.address().to_string(), m_destination.port());

        m_RepliesNumber = 0;
        m_receiveTimer.AsyncWait([&](){HandleTimeout();}, std::chrono::seconds(5));
    }

    void AsioPinger::StartReceive() {
        // Discard any data already in the buffer.
        SR_LOG("AsioPinger::StartReceive() : waiting for reply.");
        m_replyBuffer.consume(m_replyBuffer.size());

        // Wait for a reply. We prepare the buffer to receive up to 64KB.

        m_replyBuffer.prepare(65536);

        auto&& receivedSize = m_socket->AsyncReceive(&m_replyBuffer, [this](uint64_t bytesReceived){
            HandleReceive(bytesReceived);
        });
    }

    void AsioPinger::HandleReceive(uint64_t bytesReceived) {
        SR_LOG("AsioPinger::HandleReceive() : handling received bytes.");

        m_replyBuffer.commit(bytesReceived);

        // Decode the reply packet.
        std::istream is(&m_replyBuffer);
        IPV4Header ipv4_hdr;
        ICMPHeader icmp_hdr;
        is >> ipv4_hdr >> icmp_hdr;

        //if (is && icmp_hdr.Type() == SR_UTILS_NS::EnumReflector::GetIndex(ICMPHeader::EchoReply)
        if (is && icmp_hdr.Type() == static_cast<uint8_t>(ICMPType::EchoReply)
            && icmp_hdr.Identifier() == SR_PLATFORM_NS::GetCurrentProcessId()
            && icmp_hdr.SequenceNumber() == m_sequenceNumber)
        {
            if (m_RepliesNumber++ == 0) {
                ++m_successes;
                m_receiveTimer.Cancel();
            }

            std::string logMessage;
            logMessage += &"Received " [bytesReceived] - ipv4_hdr.header_length() +
                            std::string(" bytes from ") + ipv4_hdr.source_address().to_string();
            SR_LOG(logMessage);
        }

        StartReceive();
    }

    void AsioPinger::HandleTimeout() {
        if (m_RepliesNumber == 0) {
            std::cout << "AsioPinger::HandleTimeout() : request timed out" << std::endl;
        }

        m_sendTimer.AsyncWait([&](){AsioPinger::StartSend();}, std::chrono::seconds(1));
    }

    void AsioPinger::Ping(const std::string& destination) {
        asio::ip::icmp::resolver::query query(asio::ip::icmp::v4(), destination, "");

        asio::io_service io_service;
        asio::ip::icmp::resolver resolver(io_service);

        m_destination = *resolver.resolve(query);

        while (m_successes < 5) {
            StartSend();
            StartReceive();
        }
    }
}