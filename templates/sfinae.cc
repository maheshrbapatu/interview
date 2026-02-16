#include <iostream>
#include <concepts>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
T twice(T x) { return x * 2; }

int main() {
    std::cout << twice(5) << "\n";
    std::cout << twice(3.25) << "\n";
}

