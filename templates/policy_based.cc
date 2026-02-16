#include <iostream>

struct NoLock {
    void lock() {}
    void unlock() {}
};

struct PrintLock {
    void lock() { std::cout << "lock\n"; }
    void unlock() { std::cout << "unlock\n"; }
};

template <class LockPolicy>
class Counter : private LockPolicy {
    int x = 0;
public:
    void inc() {
        this->lock();
        ++x;
        this->unlock();
    }
    int value() const { return x; }
};

int main() {
    Counter<NoLock> c1; c1.inc(); std::cout << c1.value() << "\n";
    Counter<PrintLock> c2; c2.inc(); std::cout << c2.value() << "\n";
}

