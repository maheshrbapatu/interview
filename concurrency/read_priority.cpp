#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

class RWLockReaderPriority {
private:
    std::mutex m_;
    std::condition_variable cv_;
    int activeReaders_ = 0;
    bool writerActive_ = false;

public:
    void lock_read() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !writerActive_; });
        ++activeReaders_;
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(m_);
        --activeReaders_;
        if (activeReaders_ == 0) {
            cv_.notify_all(); // wake writer if waiting
        }
    }

    void lock_write() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !writerActive_ && activeReaders_ == 0; });
        writerActive_ = true;
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(m_);
        writerActive_ = false;
        cv_.notify_all(); // wake readers + writers
    }
};

// Global lock instance
RWLockReaderPriority rwlock;

// Simulated reader
void reader_function(int id) {
    for (int i = 0; i < 3; ++i) {
        rwlock.lock_read();
        std::cout << "[Reader " << id << "] reading...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        rwlock.unlock_read();

        // Small pause before trying again
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Simulated writer
void writer_function(int id, int value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // let readers start

    rwlock.lock_write();
    std::cout << ">>> [Writer " << id << "] writing value " << value << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    rwlock.unlock_write();

    std::cout << "<<< [Writer " << id << "] done\n";
}

int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // Create reader threads
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back(reader_function, i);
    }

    // Create writer threads
    for (int i = 0; i < 2; ++i) {
        writers.emplace_back(writer_function, i, i * 10);
    }

    // Join readers
    for (auto& r : readers) {
        r.join();
    }

    // Join writers
    for (auto& w : writers) {
        w.join();
    }

    std::cout << "All threads finished.\n";
    return 0;
}
