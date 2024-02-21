//
// Created by innerviewer on 2024-02-20.
//
#ifndef SR_UTILS_NETWORK_IPV4HEADER_H
#define SR_UTILS_NETWORK_IPV4HEADER_H

#include <Utils/stdInclude.h>
#include <asio/ip/address_v4.hpp>

namespace SR_NETWORK_NS {
    class IPV4Header { /// Refactor, rename to AsioIPV4Header
    public:
        IPV4Header() { std::fill(rep_, rep_ + sizeof(rep_), 0); }

        unsigned char version() const { return (rep_[0] >> 4) & 0xF; }

        unsigned short header_length() const { return (rep_[0] & 0xF) * 4; }

        unsigned char type_of_service() const { return rep_[1]; }

        unsigned short total_length() const { return decode(2, 3); }

        unsigned short identification() const { return decode(4, 5); }

        bool dont_fragment() const { return (rep_[6] & 0x40) != 0; }

        bool more_fragments() const { return (rep_[6] & 0x20) != 0; }

        unsigned short fragment_offset() const { return decode(6, 7) & 0x1FFF; }

        unsigned int time_to_live() const { return rep_[8]; }

        unsigned char protocol() const { return rep_[9]; }

        unsigned short header_checksum() const { return decode(10, 11); }

        asio::ip::address_v4 source_address() const {
            asio::ip::address_v4::bytes_type bytes{{rep_[12], rep_[13], rep_[14], rep_[15]}};
            return asio::ip::address_v4(bytes);
        }

        asio::ip::address_v4 destination_address() const {
            asio::ip::address_v4::bytes_type bytes
                    = {{rep_[16], rep_[17], rep_[18], rep_[19]}};
            return asio::ip::address_v4(bytes);
        }

        friend std::istream &operator>>(std::istream &is, IPV4Header &header) {
            is.read(reinterpret_cast<char *>(header.rep_), 20);
            if (header.version() != 4)
                is.setstate(std::ios::failbit);
            std::streamsize options_length = header.header_length() - 20;
            if (options_length < 0 || options_length > 40)
                is.setstate(std::ios::failbit);
            else
                is.read(reinterpret_cast<char *>(header.rep_) + 20, options_length);
            return is;
        }

    private:
        unsigned short decode(int a, int b) const { return (rep_[a] << 8) + rep_[b]; }

        unsigned char rep_[60];
    };
}
#endif //SR_UTILS_NETWORK_IPV4HEADER_H
