#ifndef WORKER_H
#define WORKER_H

#include <thread>
#include <vector>
#include <atomic>
#include "SafeQueue.h"
#include "FlowTable.h"

class Worker {
public:
    Worker(SafeQueue<Packet>& queue, FlowTable& flowTable);
    
    // Main loop for the worker thread
    void run();
    
    // Stop the worker (it will exit after queue is empty or shutdown signal)
    void stop();

private:
    SafeQueue<Packet>& queue_;
    FlowTable& flowTable_;
    std::atomic<bool> running_;
};

#endif // WORKER_H
