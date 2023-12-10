//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>

std::shared_mutex resource_mutex;
int value = 0; // Shared resource

void reader_function(int reader_id) {
    std::shared_lock<std::shared_mutex> lock(resource_mutex);
    // Now we have a shared lock, multiple readers can enter this section
    std::cout << "Reader " << reader_id << " reads value as " << value << std::endl;
    // Shared lock is released when lock goes out of scope
}

void writer_function(int writer_id, int new_value) {
    std::unique_lock<std::shared_mutex> lock(resource_mutex);
    // Now we have an exclusive lock, no other writer or reader can enter this section
    value = new_value;
    std::cout << "Writer " << writer_id << " writes value to " << value << std::endl;
    // Unique lock is released when lock goes out of scope
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