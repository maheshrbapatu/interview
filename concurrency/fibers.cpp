#include <boost/fiber/all.hpp>
#include <iostream>

void fiber_fn(const char* name) {
    for (int i = 0; i < 5; i++) {
        std::cout << name << " step " << i << "\n";
        boost::this_fiber::yield();   // cooperative switch to another fiber
    }
}

int main() {
    boost::fibers::fiber f1(fiber_fn, "fiber-1");
    boost::fibers::fiber f2(fiber_fn, "fiber-2");

    f1.join();
    f2.join();

    std::cout << "done\n";
    return 0;
}
