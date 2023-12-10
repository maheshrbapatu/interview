//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

std::mutex resource_mutex;
std::mutex queue_mutex;
std::condition_variable cond_var;

int active_readers = 0;
int waiting_writers = 0;
int value = 0; // Shared resource

void reader_function(int reader_id) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // Block readers if there are waiting writers to give priority to writers.
        cond_var.wait(lock, []{ return waiting_writers == 0; });
        active_readers++;
    }

    // Read the shared resource
    std::cout << "Reader " << reader_id << " reads value as " << value << std::endl;

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        active_readers--;
        // If this was the last reader, notify writers
        if (active_readers == 0) {
            cond_var.notify_all();
        }
    }
}

void writer_function(int writer_id, int new_value) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        waiting_writers++;
        // Wait until there are no active readers or writers
        cond_var.wait(lock, []{ return active_readers == 0; });
        waiting_writers--;
    }

    // Write to the shared resource
    {
        std::lock_guard<std::mutex> lock(resource_mutex);
        value = new_value;
        std::cout << "Writer " << writer_id << " writes value to " << value << std::endl;
    }

    // Notify other writers and readers that they may proceed
    cond_var.notify_all();
}

int main() {
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // Create writer threads
    for (int i = 0; i < 2; ++i) {
        writers.push_back(std::thread(writer_function, i, i * 10));
    }

    // Create reader threads
    for (int i = 0; i < 5; ++i) {
        readers.push_back(std::thread(reader_function, i));
    }

    // Join the writer threads with the main thread
    for (auto& writer : writers) {
        writer.join();
    }

    // Join the reader threads with the main thread
    for (auto& reader : readers) {
        reader.join();
    }

    return 0;
}