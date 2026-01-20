#include <iostream>
#include <thread>
#include <vector>
#include <csignal>
#include <unistd.h>
#include <algorithm> // for sort

#include "SafeQueue.h"
#include "CaptureEngine.h"
#include "FlowTable.h"
#include "Worker.h"

// Global flag for signal handling
std::atomic<bool> g_running(true);

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Stopping...\n";
    g_running = false;
}

void printMetrics(const FlowTable& flowTable, const SafeQueue<Packet>& queue, const CaptureEngine& capture, uint64_t totalProcessed) {
    // Clear screen (ANSI escape code)
    std::cout << "\033[2J\033[1;1H"; 
    
    std::cout << "=== Network Traffic Analyzer ===\n";
    std::cout << "Queue Size: " << queue.size() << "\n";
    std::cout << "Total Drops (Queue): " << queue.getDropCount() << "\n";
    std::cout << "Total Drops (Pcap): " << capture.getPcapDropCount() << "\n";
    std::cout << "--------------------------------\n";
    std::cout << "Top 5 Flows (by Packets):\n";

    auto flows = flowTable.getFlows();
    // Sort by packet count descending
    std::sort(flows.begin(), flows.end(), [](const auto& a, const auto& b) {
        return a.second.packetCount > b.second.packetCount;
    });

    int count = 0;
    for (const auto& flow : flows) {
        if (count++ >= 5) break;
        std::cout << flow.first.srcIP << ":" << flow.first.srcPort << " -> " 
                  << flow.first.dstIP << ":" << flow.first.dstPort 
                  << " [" << flow.first.protocol << "] "
                  << "Pkts: " << flow.second.packetCount 
                  << " Bytes: " << flow.second.byteCount << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string interfaceName = "eth0";
    if (argc > 1) {
        // Simple arg parsing
        std::string arg1 = argv[1];
        if (arg1 == "-i" && argc > 2) {
            interfaceName = argv[2];
        } else {
             interfaceName = arg1; // check if just the name
        }
    }

    // Register signal handler
    signal(SIGINT, signalHandler);

    // components
    const size_t MAX_QUEUE_SIZE = 10000;
    SafeQueue<Packet> packetQueue(MAX_QUEUE_SIZE);
    FlowTable flowTable;
    
    CaptureEngine captureEngine(interfaceName, packetQueue);
    
    // Workers (Consumer)
    int numWorkers = std::thread::hardware_concurrency();
    if (numWorkers == 0) numWorkers = 2;
    // Keep 1 core for capture, 1 for main/metrics
    if (numWorkers > 2) numWorkers -= 1; 

    std::vector<std::unique_ptr<Worker>> workers;
    std::vector<std::thread> workerThreads;
    
    std::cout << "Spawning " << numWorkers << " worker threads...\n";
    for (int i = 0; i < numWorkers; ++i) {
        workers.push_back(std::make_unique<Worker>(packetQueue, flowTable));
        workerThreads.emplace_back(&Worker::run, workers.back().get());
    }

    // Capture Thread (Producer)
    std::thread captureThread(&CaptureEngine::run, &captureEngine);

    // Main loop for metrics
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printMetrics(flowTable, packetQueue, captureEngine, 0);
    }

    // Shutdown sequence
    std::cout << "Shutting down components...\n";
    
    captureEngine.stop();
    if (captureThread.joinable()) captureThread.join();
    
    packetQueue.shutdown(); // Wakes up workers waiting on pop()
    
    for (auto& t : workerThreads) {
        if (t.joinable()) t.join();
    }

    std::cout << "Cleanup complete. Exiting.\n";

    return 0;
}
