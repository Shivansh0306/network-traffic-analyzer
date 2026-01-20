#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include "Common.h"

class PacketParser {
public:
    static bool parse(Packet& packet);
};

#endif // PACKET_PARSER_H
