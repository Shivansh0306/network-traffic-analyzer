#include "Worker.h"
#include "PacketParser.h"
#include <iostream>

Worker::Worker(SafeQueue<Packet>& queue, FlowTable& flowTable)
    : queue_(queue), flowTable_(flowTable), running_(false) {}

void Worker::run() {
    running_ = true;
    while (running_) {
        // Pop blocks until item available or queue shutdown
        auto packetOpt = queue_.pop();
        
        if (!packetOpt) {
            // Queue is empty and shutdown signal received
            break;
        }

        Packet packet = std::move(*packetOpt);
        
        // Parse L2/L3/L4
        if (PacketParser::parse(packet)) {
            // Update Flow Table
            flowTable_.update(packet);
        } else {
            // Invalid or non-IP packet
            // We could count these separately if needed
        }
    }
}

void Worker::stop() {
    running_ = false;
    // We rely on queue_.shutdown() being called externally to wake up the pop()
}
