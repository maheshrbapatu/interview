//
// Created by Mahesh Bapatu on 08/12/23.
//
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>

std::shared_mutex resource_mutex;
std::mutex queue_mutex;
int readers_count = 0; // Counter for the number of active readers
int value = 0; // Shared resource

void reader_function(int reader_id) {
    {
        // Lock for readers count update
        std::unique_lock<std::mutex> queue_lock(queue_mutex);
        readers_count++;
        if (readers_count == 1) {
            // First reader locks the resource_mutex
            resource_mutex.lock_shared();
        }
    }

    // Reading is performed
    std::cout << "Reader " << reader_id << " reads value as " << value << std::endl;

    {
        // Unlock for readers count update
        std::unique_lock<std::mutex> queue_lock(queue_mutex);
        readers_count--;
        if (readers_count == 0) {
            // Last reader unlocks the resource_mutex
            resource_mutex.unlock_shared();
        }
    }
    // Shared lock is released when lock goes out of scope
}

void writer_function(int writer_id, int new_value) {
    // Writers will wait here if the resource_mutex is locked by readers
    std::unique_lock<std::shared_mutex> lock(resource_mutex);
    // Writing is performed
    value = new_value;
    std::cout << "Writer " << writer_id << " writes value to " << value << std::endl;
}

int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // Create reader threads
    for (int i = 0; i < 5; ++i) {
        readers.push_back(std::thread(reader_function, i));
    }

    // Create writer threads
    for (int i = 0; i < 2; ++i) {
        writers.push_back(std::thread(writer_function, i, i * 10));
    }

    // Join the reader threads with the main thread
    for (auto& reader : readers) {
        reader.join();
    }

    // Join the writer threads with the main thread
    for (auto& writer : writers) {
        writer.join();
    }

    return 0;
}