#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

class RWLockWriterPriority {
private:
    std::mutex m_;
    std::condition_variable cv_;
    int activeReaders_ = 0;
    bool writerActive_ = false;
    int waitingWriters_ = 0; // key for writer priority

public:
    void lock_read() {
        std::unique_lock<std::mutex> lk(m_);
        // Writer priority: if any writer is waiting, block new readers
        cv_.wait(lk, [&] { return !writerActive_ && waitingWriters_ == 0; });
        ++activeReaders_;
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(m_);
        --activeReaders_;
        if (activeReaders_ == 0) {
            cv_.notify_all();
        }
    }

    void lock_write() {
        std::unique_lock<std::mutex> lk(m_);
        ++waitingWriters_;
        cv_.wait(lk, [&] { return !writerActive_ && activeReaders_ == 0; });
        --waitingWriters_;
        writerActive_ = true;
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(m_);
        writerActive_ = false;
        cv_.notify_all();
    }
};

// Global lock instance
RWLockWriterPriority rwlock;

// Simulated reader: will get blocked once writers show up (writer priority)
void reader_function(int id) {
    for (int i = 0; i < 5; ++i) {
        rwlock.lock_read();
        std::cout << "[Reader " << id << "] reading...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        rwlock.unlock_read();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Simulated writer: writers should cut in front of new readers
void writer_function(int id, int value) {
    // Stagger writers so they arrive while readers are active
    std::this_thread::sleep_for(std::chrono::milliseconds(150 + id * 100));

    rwlock.lock_write();
    std::cout << ">>> [Writer " << id << "] writing value " << value << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    rwlock.unlock_write();

    std::cout << "<<< [Writer " << id << "] done\n";
}

int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // Start readers first
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back(reader_function, i);
    }

    // Start writers
    for (int i = 0; i < 3; ++i) {
        writers.emplace_back(writer_function, i, i * 10);
    }

    for (auto& r : readers) r.join();
    for (auto& w : writers) w.join();

    std::cout << "All threads finished.\n";
    return 0;
}
