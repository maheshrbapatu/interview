//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <atomic>

class RWLockFairFIFO {
private:
    // Turnstile: everyone (readers + writers) queues here in arrival order.
    std::mutex queue_;

    // Protects state below
    std::mutex m_;
    std::condition_variable cv_;
    int activeReaders_ = 0;
    bool writerActive_ = false;

public:
    void lock_read() {
        // 1) Line up fairly
        std::unique_lock<std::mutex> qlk(queue_);

        // 2) Wait only for an active writer (no writer barging)
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !writerActive_; });

        ++activeReaders_;

        // 3) Release turnstile early so next thread can queue/wait.
        //    This allows batching of readers *that arrived behind a reader*.
        qlk.unlock();
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(m_);
        --activeReaders_;
        if (activeReaders_ == 0) {
            cv_.notify_all(); // wake a waiting writer (or readers)
        }
    }

    void lock_write() {
        // 1) Line up fairly
        std::unique_lock<std::mutex> qlk(queue_);

        // 2) Wait for exclusivity
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !writerActive_ && activeReaders_ == 0; });
        writerActive_ = true;

        // 3) Release turnstile so next thread can start waiting
        qlk.unlock();
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(m_);
        writerActive_ = false;
        cv_.notify_all();
    }
};

// Shared state
RWLockFairFIFO rw;
int sharedValue = 0;
std::atomic<bool> stopFlag{false};

void reader_fn(int id) {
    while (!stopFlag.load()) {
        rw.lock_read();
        int v = sharedValue;
        std::cout << "[R" << id << "] read sharedValue=" << v << "\n";
        rw.unlock_read();

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void writer_fn(int id, int times) {
    for (int i = 0; i < times; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(120));

        rw.lock_write();
        int before = sharedValue;
        sharedValue = before + 1;
        std::cout << ">>> [W" << id << "] wrote " << before << " -> " << sharedValue << "\n";
        rw.unlock_write();
    }
}

int main() {
    std::vector<std::thread> readers, writers;

    // Start readers
    for (int i = 0; i < 4; ++i) readers.emplace_back(reader_fn, i);

    // Start writers
    for (int i = 0; i < 2; ++i) writers.emplace_back(writer_fn, i, 6);

    for (auto& w : writers) w.join();

    stopFlag.store(true);
    for (auto& r : readers) r.join();

    std::cout << "Final sharedValue=" << sharedValue << "\n";
    return 0;
}
