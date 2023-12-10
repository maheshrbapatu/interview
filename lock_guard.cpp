//
// Created by Mahesh Bapatu on 10/12/23.
//

//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// A shared mutex to protect access to shared data
std::mutex mtx1;
// A shared variable that will be modified by threads
int shared_data = 0;

// A function that increments shared data
void increment_shared_data() {
    // Using The constructor of std::lock_guard to lock one mutex at once, for multiple use scoped lock.
    // C++ 11 - only one lock
    std::lock_guard lock(mtx1);
    // Increment shared data
    std::cout << "Thread " << std::this_thread::get_id() << " is incrementing shared_data." << std::endl;
    ++shared_data;
    // Once the function add_count finishes, the object lock is out of scope, and
    // in its destructor, the mutex m is released.
}

int main() {
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
