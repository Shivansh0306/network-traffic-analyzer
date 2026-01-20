#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <chrono>
#include <cstdint>
#include <string>

// High-resolution timestamp
using TimePoint = std::chrono::high_resolution_clock::time_point;

struct Packet {
    std::vector<uint8_t> payload;
    TimePoint timestamp;
    
    // Metadata filled by parser
    std::string srcIP;
    std::string dstIP;
    uint16_t srcPort = 0;
    uint16_t dstPort = 0;
    std::string protocol;
    uint32_t length = 0;

    Packet() : timestamp(std::chrono::high_resolution_clock::now()) {}
};

#endif // COMMON_H
