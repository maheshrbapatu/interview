#include <iostream>

template <typename T>
T add(T a, T b) { return a + b; }

int main() {
    std::cout << add(2, 3) << "\n";
    std::cout << add(1.5, 2.5) << "\n";
}

