#ifndef FLOW_TABLE_H
#define FLOW_TABLE_H

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <functional> // For std::hash
#include "Common.h"

struct FlowKey {
    std::string srcIP;
    std::string dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
    std::string protocol;

    bool operator==(const FlowKey& other) const {
        return srcIP == other.srcIP && dstIP == other.dstIP &&
               srcPort == other.srcPort && dstPort == other.dstPort &&
               protocol == other.protocol;
    }
};

struct FlowStats {
    uint64_t packetCount = 0;
    uint64_t byteCount = 0;
};

// Custom Hash for FlowKey
struct FlowKeyHash {
    std::size_t operator()(const FlowKey& k) const {
        // Simple combination of hashes
        return std::hash<std::string>()(k.srcIP) ^ 
               (std::hash<std::string>()(k.dstIP) << 1) ^ 
               (std::hash<uint16_t>()(k.srcPort) << 1) ^ 
               (std::hash<uint16_t>()(k.dstPort) << 1) ^
               std::hash<std::string>()(k.protocol);
    }
};

class FlowTable {
public:
    void update(const Packet& packet);
    
    // Returns a snapshot of flow stats. 
    // For sorting top N, we return vector of pairs.
    std::vector<std::pair<FlowKey, FlowStats>> getFlows() const;

private:
    std::unordered_map<FlowKey, FlowStats, FlowKeyHash> table_;
    mutable std::shared_mutex mutex_;
};

#endif // FLOW_TABLE_H
