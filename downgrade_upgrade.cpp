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

class UpgradableRWLock {
private:
    std::mutex m_;
    std::condition_variable cv_;

    int activeReaders_ = 0;       // number of shared readers holding the lock
    bool writerActive_ = false;   // a writer currently holds the lock

    bool upgraderActive_ = false; // at most one "upgradeable reader" at a time
    int waitingWriters_ = 0;      // optional: helps avoid writer starvation

public:
    // ----- Shared Read -----
    void lock_read() {
        std::unique_lock<std::mutex> lk(m_);
        // Block if a writer is active or an upgrader is trying to upgrade? (not necessary)
        // For cleanliness: block only on active writer.
        cv_.wait(lk, [&] { return !writerActive_; });
        ++activeReaders_;
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(m_);
        --activeReaders_;
        if (activeReaders_ == 0) cv_.notify_all();
    }

    // ----- Upgradeable Read (only one upgrader at a time) -----
    void lock_upgrade() {
        std::unique_lock<std::mutex> lk(m_);
        // Wait until no writer is active, and no other upgrader exists.
        // (We allow other readers concurrently.)
        cv_.wait(lk, [&] { return !writerActive_ && !upgraderActive_; });
        upgraderActive_ = true;
        ++activeReaders_; // upgrader also counts as a reader while in upgrade mode
    }

    void unlock_upgrade() {
        std::unique_lock<std::mutex> lk(m_);
        // Still holding read share, just like a reader
        --activeReaders_;
        upgraderActive_ = false;
        if (activeReaders_ == 0) cv_.notify_all();
        else cv_.notify_all(); // allow waiting upgrader/writer to re-check
    }

    // Convert upgradeable-read -> write
    // Precondition: caller holds "upgrade lock" (i.e., lock_upgrade() was called and not released).
    void upgrade_to_write() {
        std::unique_lock<std::mutex> lk(m_);

        // We are currently counted in activeReaders_ as one reader.
        // To become a writer, we must be the ONLY reader and no writer active.
        ++waitingWriters_;
        cv_.wait(lk, [&] {
            return !writerActive_ && activeReaders_ == 1; // only "me" remains
        });
        --waitingWriters_;

        // Drop our reader share and become writer.
        --activeReaders_;
        writerActive_ = true;

        // We keep upgraderActive_ = true? Two reasonable choices:
        // - Keep it true until we downgrade/unlock, so no other upgrader enters.
        // This is simplest and avoids odd interleavings.
    }

    // Convert write -> read (downgrade)
    // Precondition: caller holds write lock (writerActive_ == true for this thread)
    // Postcondition: caller holds a shared read lock (and upgrader slot is released).
    void downgrade_to_read() {
        std::unique_lock<std::mutex> lk(m_);
        // Become a reader first (so there is no gap where nobody holds state)
        ++activeReaders_;

        // Release writer exclusivity
        writerActive_ = false;

        // If we came from an upgrade path, release the upgrader slot now
        if (upgraderActive_) upgraderActive_ = false;

        cv_.notify_all();
    }

    // ----- Exclusive Write -----
    void lock_write() {
        std::unique_lock<std::mutex> lk(m_);
        ++waitingWriters_;

        // Writer-priority-ish: block readers if writers are waiting (handled in lock_read only if you want).
        // Here: wait for no writer and no readers and no upgrader.
        cv_.wait(lk, [&] {
            return !writerActive_ && activeReaders_ == 0 && !upgraderActive_;
        });

        --waitingWriters_;
        writerActive_ = true;
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(m_);
        writerActive_ = false;
        cv_.notify_all();
    }
};

// ---------------- Demo ----------------
UpgradableRWLock rw;
int sharedValue = 0;
std::atomic<bool> stopFlag{false};

void reader_thread(int id) {
    while (!stopFlag.load()) {
        rw.lock_read();
        int v = sharedValue;
        std::cout << "[R" << id << "] read " << v << "\n";
        rw.unlock_read();

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void writer_thread(int id, int iters) {
    for (int i = 0; i < iters; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        rw.lock_write();
        int before = sharedValue;
        sharedValue = before + 10;
        std::cout << ">>> [W" << id << "] wrote " << before << " -> " << sharedValue << "\n";
        rw.unlock_write();
    }
}

void upgrader_thread(int id) {
    // read -> upgrade -> write -> downgrade -> read
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    rw.lock_upgrade();
    std::cout << "[U" << id << "] upgrade-read sees " << sharedValue << "\n";

    // simulate doing some work while shared-reading
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "[U" << id << "] attempting upgrade_to_write...\n";
    rw.upgrade_to_write(); // now exclusive
    int before = sharedValue;
    sharedValue = before + 1;
    std::cout << ">>> [U" << id << "] upgraded & wrote " << before << " -> " << sharedValue << "\n";

    // keep exclusive for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    std::cout << "[U" << id << "] downgrading to read...\n";
    rw.downgrade_to_read(); // now shared read
    std::cout << "[U" << id << "] after downgrade read sees " << sharedValue << "\n";
    rw.unlock_read(); // because downgrade_to_read gives you a read lock

    // done
}

int main() {
    std::vector<std::thread> threads;

    // Readers
    for (int i = 0; i < 3; ++i) threads.emplace_back(reader_thread, i);

    // One upgrader
    threads.emplace_back(upgrader_thread, 0);

    // Writers
    threads.emplace_back(writer_thread, 0, 3);
    threads.emplace_back(writer_thread, 1, 3);

    // Wait writers/upgrader, then stop readers
    for (auto& t : threads) {
        // we’ll stop readers after writers finish; simplest: join all non-reader first
        // but for quick demo, just join all and stop after a while.
        // (Instead we’ll run for a short time and stop.)
    }

    // Run for a bit then stop readers
    std::this_thread::sleep_for(std::chrono::seconds(3));
    stopFlag.store(true);

    for (auto& t : threads) t.join();

    std::cout << "Final sharedValue = " << sharedValue << "\n";
    return 0;
}
