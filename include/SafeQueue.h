#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include "Common.h"

template <typename T>
class SafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    size_t maxSize_;
    std::atomic<bool> shutdown_;
    std::atomic<size_t> dropCount_;

public:
    SafeQueue(size_t maxSize) : maxSize_(maxSize), shutdown_(false), dropCount_(0) {}

    // Returns true if pushed, false if dropped (queue full)
    bool push(T item) {
        if (shutdown_) return false;

        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() >= maxSize_) {
            dropCount_++;
            return false;
        }

        queue_.push(std::move(item));
        lock.unlock();
        cond_.notify_one();
        return true;
    }

    // Blocks until item is available or shutdown
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty() || shutdown_; });

        if (queue_.empty() && shutdown_) {
            return std::nullopt;
        }

        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    void shutdown() {
        shutdown_ = true;
        cond_.notify_all();
    }

    size_t getDropCount() const {
        return dropCount_.load();
    }

    size_t size() const {
        // Approximate size without locking for stats
        return queue_.size(); 
    }
};

#endif // SAFE_QUEUE_H
