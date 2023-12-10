//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>

// Mutexes and condition variable for synchronizing access to the shared resource
std::mutex resource_mutex;
std::mutex queue_mutex;
std::condition_variable cv;

// FIFO queue to ensure that threads get access to the resource in the order they come in
std::queue<std::thread::id> fifo_queue;

// Shared resource which is to be accessed by readers and writers
int value = 0;

// Flag to indicate if a writer is currently active
bool is_writer_active = false;

// Function for threads to wait in queue in FIFO order
void queue_wait(std::unique_lock<std::mutex>& lock) {
    auto current_id = std::this_thread::get_id(); // Get the ID of the current thread
    // fifo_queue.front() == current_id checks whether the current thread's ID is at the front of the queue.
    // Only the thread at the front should be allowed to proceed to ensure FIFO (First-In-First-Out) order.
    //is_writer_active is a check to ensure that there is no writer currently writing to the shared resource.
    // If a writer is active (is_writer_active is true), then readers must wait, regardless of their position in
    // the queue
    fifo_queue.push(current_id); // Add the thread ID to the queue

    // Wait until the thread is at the front of the queue and no writer is active
    cv.wait(lock, [&] { return fifo_queue.front() == current_id && !is_writer_active; });
    fifo_queue.pop(); // Remove the thread from the queue
}

// Function representing the reader's actions
void reader_function(int reader_id) {
    {
        // Lock the queue mutex to safely enter the FIFO queue
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_wait(lock); // Wait in the queue in FIFO order
    }

    // Reading is performed outside the queue mutex to allow other readers to queue up
    std::cout << "Reader " << reader_id << " reads value as " << value << std::endl;

    // Notify all waiting threads that this thread has finished reading
    cv.notify_all();
}

// Function representing the writer's actions
void writer_function(int writer_id, int new_value) {
    {
        // Lock the queue mutex to safely enter the FIFO queue
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_wait(lock); // Wait in the queue in FIFO order
    }

    // Once the thread is at the front of the queue, set the writer flag
    is_writer_active = true;

    // Write the new value to the shared resource
    value = new_value;
    std::cout << "Writer " << writer_id << " writes value to " << value << std::endl;

    // After writing, clear the writer flag
    is_writer_active = false;

    // Notify all waiting threads that this thread has finished writing
    cv.notify_all();
}

int main() {
    std::vector<std::thread> readers; // Vector to hold reader threads
    std::vector<std::thread> writers; // Vector to hold writer threads

    // Create and start reader threads
    for (int i = 0; i < 5; ++i) {
        readers.push_back(std::thread(reader_function, i));
    }

    // Create and start writer threads
    for (int i = 0; i < 2; ++i) {
        writers.push_back(std::thread(writer_function, i, i * 10));
    }

    // Wait for all reader threads to finish
    for (auto& reader : readers) {
        reader.join();
    }

    // Wait for all writer threads to finish
    for (auto& writer : writers) {
        writer.join();
    }

    return 0; // End of the program
}