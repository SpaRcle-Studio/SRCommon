//
// Created by innerviewer on 2024-02-20.
//
#ifndef SR_UTILS_NETWORK_ASIOPINGER_H
#define SR_UTILS_NETWORK_ASIOPINGER_H

#include <Utils/Network/Pinger.h>

#include <asio/ip/icmp.hpp>
#include <asio/streambuf.hpp>

namespace SR_NETWORK_NS {
    class AsioPinger : public SR_NETWORK_NS::Pinger {
        using Super = SR_NETWORK_NS::Pinger;
    public:
        AsioPinger(const std::string& destination);

    public:
        void Ping() override { }

    private:
        void StartSend();
        void StartReceive();
        void HandleTimeout();
        void HandleReceive(std::size_t length);

    private:
        asio::ip::icmp::endpoint m_destination;
        asio::ip::icmp::resolver m_resolver;
        /// TODO: TIMER!!!!!    SR_HTYPES_NS::DeadlineTimer m_timer;
        SR_UTILS_NS::TimePointType m_sendTime;
        asio::streambuf m_replyBuffer;
        uint16_t m_sequenceNumber = 0;
        uint16_t m_RepliesNumber = 0;
    };
}
#endif //SR_UTILS_NETWORK_ASIOPINGER_H
