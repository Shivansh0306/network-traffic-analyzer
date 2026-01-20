#include "CaptureEngine.h"
#include <iostream>
#include <cstring>

CaptureEngine::CaptureEngine(const std::string& interfaceName, SafeQueue<Packet>& packetQueue)
    : interfaceName_(interfaceName), packetQueue_(packetQueue), running_(false), pcapHandle_(nullptr) {
}

CaptureEngine::~CaptureEngine() {
    stop();
    if (pcapHandle_) {
        pcap_close(pcapHandle_);
    }
}

void CaptureEngine::run() {
    // Open the session in promiscuous mode
    // buffer size 65535, timeout 1ms (to allow checking running_ flag or interrupt)
    pcapHandle_ = pcap_open_live(interfaceName_.c_str(), 65535, 1, 1, errbuf_);
    
    if (pcapHandle_ == nullptr) {
        std::cerr << "Error opening interface " << interfaceName_ << ": " << errbuf_ << std::endl;
        return;
    }

    // Compile filter (optional, default empty for all traffic)
    // For now we capture everything.

    running_ = true;
    std::cout << "Starting capture on " << interfaceName_ << "..." << std::endl;

    // Use pcap_loop. We pass 'this' as user data to access the queue.
    // Loop continues until error, stop, or pcap_breakloop.
    while (running_) {
        // dispatch 1 packet at a time to allow frequent checks if needed, 
        // but pcap_loop is better. We can use pcap_loop with -1 and pcap_breakloop to stop.
        int ret = pcap_loop(pcapHandle_, -1, packetHandler, reinterpret_cast<u_char*>(this));
        if (ret == -2) { // pcap_breakloop called
             std::cout << "Capture loop broken." << std::endl;
             break;
        } else if (ret == -1) {
             std::cerr << "Error in pcap_loop: " << pcap_geterr(pcapHandle_) << std::endl;
             break;
        }
    }
    
    std::cout << "Capture stopped." << std::endl;
}

void CaptureEngine::stop() {
    running_ = false;
    if (pcapHandle_) {
        pcap_breakloop(pcapHandle_);
    }
}

void CaptureEngine::packetHandler(u_char* user, const struct pcap_pkthdr* pkthdr, const u_char* packetData) {
    CaptureEngine* engine = reinterpret_cast<CaptureEngine*>(user);
    if (!engine->running_) return;

    Packet packet;
    // Copy data to vector
    packet.payload.assign(packetData, packetData + pkthdr->caplen);
    packet.length = pkthdr->len; // Original length (wire length)
    packet.timestamp = std::chrono::high_resolution_clock::now(); // Timestamp

    // Push to queue
    // If queue is full, push returns false and increments drop count inside queue
    engine->packetQueue_.push(std::move(packet));
}

uint64_t CaptureEngine::getPcapDropCount() const {
    // In a real scenario, we might query pcap_stats here
    if (pcapHandle_) {
        struct pcap_stat ps;
        if (pcap_stats(pcapHandle_, &ps) == 0) {
            return ps.ps_drop;
        }
    }
    return 0;
}
