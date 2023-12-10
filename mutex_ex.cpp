//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

// A shared mutex to protect access to shared data
std::mutex mtx;

// A shared variable that will be modified by threads
int shared_data = 0;

// A function that increments shared data
void increment_shared_data() {
    // Lock the mutex to ensure exclusive access to shared_data
    mtx.lock();

    // Increment shared data
    std::cout << "Thread " << std::this_thread::get_id() << " is incrementing shared_data." << std::endl;
    ++shared_data;
    // Unlock the mutex
    mtx.unlock();
}

int main() {
    // Create a number of threads that will operate on shared_data
    //  calling emplace_back inside a loop, you might be causing
    //  multiple reallocations of the vector as it grows to accommodate
    //  new elements. This can lead to inefficient memory usage and potentially
    //  slow down your program due to the repeated allocations and copying/moving
    //  of elements.
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment_shared_data);
    }

    // Join all the threads to the main thread
    for (auto& thread : threads) {
        thread.join();
    }

    // Output the result
    std::cout << "shared_data: " << shared_data << std::endl;

    return 0;
}