#ifndef CAPTURE_ENGINE_H
#define CAPTURE_ENGINE_H

#include <string>
#include <atomic>
#include <pcap.h>
#include "SafeQueue.h"
#include "Common.h"

class CaptureEngine {
public:
    CaptureEngine(const std::string& interfaceName, SafeQueue<Packet>& packetQueue);
    ~CaptureEngine();

    // Starts the capture loop (blocking)
    void run();

    // Signal to stop capturing
    void stop();

    uint64_t getPcapDropCount() const;

private:
    std::string interfaceName_;
    SafeQueue<Packet>& packetQueue_;
    std::atomic<bool> running_;
    pcap_t* pcapHandle_;
    char errbuf_[PCAP_ERRBUF_SIZE];

    static void packetHandler(u_char* user, const struct pcap_pkthdr* pkthdr, const u_char* packet);
};

#endif // CAPTURE_ENGINE_H
