//
// Created by innerviewer on 2024-02-20.
//
#ifndef SR_UTILS_NETWORK_ICMPHEADER_H
#define SR_UTILS_NETWORK_ICMPHEADER_H

#include <Utils/Common/Enumerations.h>

namespace SR_NETWORK_NS {
    SR_ENUM_NS_CLASS_T(ICMPType, uint8_t,
           EchoReply = 0,
           DestinationUnreachable = 3,
           SourceQuench = 4,
           Redirect = 5,
           EchoRequest = 8,
           TimeExceeded = 11,
           ParameterProblem = 12,
           TimestampRequest = 13,
           TimestampReply = 14,
           InfoRequest = 15,
           InfoReply = 16,
           AddressRequest = 17,
           AddressReply = 18
    );

    class ICMPHeader {
    public:
        ICMPHeader() { std::fill(rep_, rep_ + sizeof(rep_), 0); }

        unsigned char Type() const { return rep_[0]; }
        unsigned char Code() const { return rep_[1]; }
        uint16_t Checksum() const { return Decode(2, 3); }
        uint16_t Identifier() const { return Decode(4, 5); }
        uint16_t SequenceNumber() const { return Decode(6, 7); }

        void Type(ICMPTypeFlag type) { rep_[0] = type; }
        void Code(ICMPTypeFlag type) { rep_[1] = type; }
        void Checksum(ICMPTypeFlag type) { Encode(2, 3, type); }
        void Identifier(ICMPTypeFlag type) { Encode(4, 5, type); }
        void SequenceNumber(ICMPTypeFlag type) { Encode(6, 7, type); }

        friend std::istream& operator>>(std::istream& is, ICMPHeader& header)
        { return is.read(reinterpret_cast<char*>(header.rep_), 8); }

        friend std::ostream& operator<<(std::ostream& os, const ICMPHeader& header)
        { return os.write(reinterpret_cast<const char*>(header.rep_), 8); }

    private:
        uint16_t Decode(int a, int b) const
        { return (rep_[a] << 8) + rep_[b]; }

        void Encode(int a, int b, uint16_t n)
        {
            rep_[a] = static_cast<unsigned char>(n >> 8);
            rep_[b] = static_cast<unsigned char>(n & 0xFF);
        }

        unsigned char rep_[8];
    };

    template <typename Iterator>
    void ComputeChecksum(ICMPHeader& header,
                          Iterator body_begin, Iterator body_end)
    {
        uint32_t sum = (header.Type() << 8) + header.Code()
                           + header.Identifier() + header.SequenceNumber();

        Iterator body_iter = body_begin;
        while (body_iter != body_end)
        {
            sum += (static_cast<unsigned char>(*body_iter++) << 8);
            if (body_iter != body_end)
                sum += static_cast<unsigned char>(*body_iter++);
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        header.Checksum(static_cast<uint16_t>(~sum));
    };
}

#endif //SR_UTILS_NETWORK_ICMPHEADER_H
