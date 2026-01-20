#include "FlowTable.h"

void FlowTable::update(const Packet& packet) {
    FlowKey key{packet.srcIP, packet.dstIP, packet.srcPort, packet.dstPort, packet.protocol};
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    FlowStats& stats = table_[key];
    stats.packetCount++;
    stats.byteCount += packet.length;
}

std::vector<std::pair<FlowKey, FlowStats>> FlowTable::getFlows() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::pair<FlowKey, FlowStats>> flows;
    flows.reserve(table_.size());
    for (const auto& kv : table_) {
        flows.push_back(kv);
    }
    return flows;
}
