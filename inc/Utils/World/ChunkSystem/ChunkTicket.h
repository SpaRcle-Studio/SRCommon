//
// Created by innerviewer on 2024-04-28.
//
#ifndef SR_COMMON_CHUNK_TICKET_H
#define SR_COMMON_CHUNK_TICKET_H

namespace SR_WORLD_NS {
    SR_ENUM_NS_CLASS_T(ChunkTicketType, uint16_t,
       Observer,
       Forced,
       Start,
       Unknown
   )

    /*class ChunkTicket {
    public:
        ChunkTicket() = default;
        ChunkTicket(uint16_t level, TicketType type, float_t timeToLive);
        ChunkTicket(uint16_t level, TicketType type);
        ~ChunkTicket() = default;

    public:
        uint16_t m_level = 10;
        TicketType m_type = TicketType::Unknown;
        float_t m_timeToLive = 0;
    };*/

    struct ChunkTicket {
        uint16_t level = 10;
        ChunkTicketType type = ChunkTicketType::Unknown;
        float_t timeToLive = SR_FLOAT_MAX;

        static bool Compare(const ChunkTicket& left, ChunkTicket right) {
            return left.level < right.level;
        }
    };
}

#endif //SR_COMMON_CHUNK_TICKET_H
