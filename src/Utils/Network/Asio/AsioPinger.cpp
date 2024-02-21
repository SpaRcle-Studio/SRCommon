//
// Created by innerviewer on 2024-02-20.
//

#include <Utils/Network/Asio/AsioPinger.h>
#include <Utils/Network/Asio/AsioContext.h>

#include <Utils/Network/Headers/ICMPHeader.h>
#include <Utils/Network/Headers/IPV4Header.h>

#include <asio/basic_deadline_timer.hpp>

#include <Utils/Platform/Platform.h>

#include <Utils/Common/EnumReflector.h>

namespace SR_NETWORK_NS {
    AsioPinger::AsioPinger(const std::string& destination)
        // Nikita, resolver does not have a default constructor
        : m_resolver(asio::ip::icmp::resolver(m_context.DynamicCast<AsioContext>()->GetContext()))
    {
        m_socket = m_context->CreateSocket(SocketType::ICMP);

        auto&& connectionResult = m_socket->Connect(destination, 0);

        asio::ip::icmp::resolver::query query(asio::ip::icmp::v4(), destination, "");
        m_destination = *m_resolver.resolve(query);

        StartSend();
        StartReceive();
    }

    void AsioPinger::StartSend() {
        std::string body("\"Hello!\" from SpaRcle Network ping.");

        // Create an ICMP header for an echo request.
        ICMPHeader echoRequest;
        echoRequest.Type(SR_UTILS_NS::EnumReflector::GetIndex(ICMPType::EchoRequest));
        echoRequest.Code(0);
        echoRequest.Identifier(SR_PLATFORM_NS::GetCurrentProcessId());
        echoRequest.SequenceNumber(++m_sequenceNumber);
        ComputeChecksum(echoRequest, body.begin(), body.end());

        // Encode the request packet.
        asio::streambuf requestBuffer;
        std::ostream os(&requestBuffer);
        os << echoRequest << body;

        // Send the request.
        m_sendTime = SR_UTILS_NS::TimePointType::clock::now();
        auto&& sendResult = m_socket->Send(requestBuffer.data().data(), requestBuffer.size());

        // Wait up to five seconds for a reply.
        m_RepliesNumber = 0;
        ///m_timer.expires_at(time_sent_ + posix_time::seconds(5));
        ///m_times.async_wait(boost::bind(&pinger::handle_timeout, this));
    }

    void AsioPinger::StartReceive() {
        // Discard any data already in the buffer.
        m_replyBuffer.consume(m_replyBuffer.size());

        // Wait for a reply. We prepare the buffer to receive up to 64KB.
        //m_socket.async_receive(m_replyBuffer.prepare(65536),
        //                      boost::bind(&pinger::handle_receive, this, _2));
    }

    void AsioPinger::HandleReceive(std::size_t length) {
        // The actual number of bytes received is committed to the buffer so that we
        // can extract it using a std::istream object.
        m_replyBuffer.commit(length);

        // Decode the reply packet.
        std::istream is(&m_replyBuffer);
        IPV4Header ipv4_hdr;
        ICMPHeader icmp_hdr;
        is >> ipv4_hdr >> icmp_hdr;

        // We can receive all ICMP packets received by the host, so we need to
        // filter out only the echo replies that match the our identifier and
        // expected sequence number.
        if (is && icmp_hdr.Type() == SR_UTILS_NS::EnumReflector::GetIndex(ICMPType::EchoReply)
            && icmp_hdr.Identifier() == SR_PLATFORM_NS::GetCurrentProcessId()
            && icmp_hdr.SequenceNumber() == m_sequenceNumber)
        {
            // If this is the first reply, interrupt the five second timeout.
            if (m_RepliesNumber++ == 0) {
                //timer_.cancel(); /// TODO: TIMER!!!!!!!!!!!!!!!
            }
            // Print out some information about the reply packet.
            //posix_time::ptime now = posix_time::microsec_clock::universal_time();

            std::string logMessage;
            logMessage += &"Recieved " [ length] - ipv4_hdr.header_length() +
                            std::string(" bytes from ") + ipv4_hdr.source_address().to_string();
            SR_LOG(logMessage);


            /*std::cout << length - ipv4_hdr.header_length()
                      << " bytes from " << ipv4_hdr.source_address()
                      << ": icmp_seq=" << icmp_hdr.sequence_number()
                      << ", ttl=" << ipv4_hdr.time_to_live()
                      << ", time=" << (now - time_sent_).total_milliseconds() << " ms"
                      << std::endl;*/
        }

        StartReceive();
    }

    void AsioPinger::HandleTimeout() {
        /*if (num_replies_ == 0)
            std::cout << "Request timed out" << std::endl;

        // Requests must be sent no less than one second apart.
        timer_.expires_at(time_sent_ + posix_time::seconds(1));
        timer_.async_wait(boost::bind(&pinger::start_send, this));*/
    }
}