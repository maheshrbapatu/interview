#include <future>
#include <iostream>

int work() {
    return 10;
}

int main() {
    auto f = std::async(work);
    std::cout << f.get();  // waits + returns value
}
