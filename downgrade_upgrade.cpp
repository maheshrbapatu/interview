//
// Created by Mahesh Bapatu on 10/12/23.
//
/*
 * ReadWriteLock with Upgrade and Downgrade: Some implementations of read-write locks
 * allow a reader to "upgrade" to a writer (if they decide they need to write) and a writer to "downgrade" to a reader
 * (after they're done writing). This can be useful for reducing lock contention when the role of a thread may change
 * over time.
 */
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>

std::shared_mutex resource_mutex;
int value = 0; // Shared resource

void reader_function(int reader_id) {
    std::shared_lock<std::shared_mutex> lock(resource_mutex);
    // Reading the shared resource with shared lock
    std::cout << "Reader " << reader_id << " reads value as " << value << std::endl;
    // Shared lock is released when lock goes out of scope
}

void writer_function(int writer_id, int new_value) {
    std::unique_lock<std::shared_mutex> lock(resource_mutex);
    // Writing to the shared resource with exclusive lock
    value = new_value;
    std::cout << "Writer " << writer_id << " writes value to " << value << std::endl;
    // Exclusive lock is released when lock goes out of scope
}

// Function to upgrade from reader to writer
void upgrade_function(int reader_id) {
    // First, acquire a shared lock
    std::shared_lock<std::shared_mutex> shared_lock(resource_mutex);
    std::cout << "Reader " << reader_id << " preparing to upgrade." << std::endl;

    // Upgrade: release shared lock and acquire exclusive lock
    shared_lock.unlock();
    std::unique_lock<std::shared_mutex> exclusive_lock(resource_mutex);

    // Now we have exclusive access
    value *= 2; // Example operation: double the value
    std::cout << "Upgrader " << reader_id << " upgraded and doubled value to " << value << std::endl;

    // Exclusive lock will be released when going out of scope
}

// Function to downgrade from writer to reader
void downgrade_function(int writer_id, int new_value) {
    // First, acquire an exclusive lock
    std::unique_lock<std::shared_mutex> exclusive_lock(resource_mutex);
    value = new_value;
    std::cout << "Writer " << writer_id << " preparing to downgrade after writing value to " << value << std::endl;

    // Downgrade: release exclusive lock and acquire shared lock
    exclusive_lock.unlock();
    std::shared_lock<std::shared_mutex> shared_lock(resource_mutex);

    // Now we have shared access
    std::cout << "Downgrader " << writer_id << " downgraded and reads value as " << value << std::endl;

    // Shared lock will be released when going out of scope
}

int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;
    std::vector<std::thread> upgraders;
    std::vector<std::thread> downgraders;

    // Create reader threads
    for (int i = 0; i < 5; ++i) {
        readers.push_back(std::thread(reader_function, i));
    }

    // Create writer and upgrade threads
    for (int i = 0; i < 2; ++i) {
        writers.push_back(std::thread(writer_function, i, i * 10));
        upgraders.push_back(std::thread(upgrade_function, i));
    }

    // Create downgrader threads
    for (int i = 0; i < 2; ++i) {
        downgraders.push_back(std::thread(downgrade_function, i, i * 100));
    }

    // Join the threads with the main thread
    for (auto& reader : readers) {
        reader.join();
    }
    for (auto& writer : writers) {
        writer.join();
    }
    for (auto& upgrader : upgraders) {
        upgrader.join();
    }
    for (auto& downgrader : downgraders) {
        downgrader.join();
    }

    return 0;
}