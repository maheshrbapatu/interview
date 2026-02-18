#include <iostream>
#include <concepts>
#include <vector>
#include <string>

// --------------------------------------------------
// 1. Built-in Concept – Integral
// Only integers allowed
// --------------------------------------------------
template <std::integral T>
T add(T a, T b) {
    return a + b;
}

// --------------------------------------------------
// 2. Built-in Concept – Floating Point
// Using requires clause style
// --------------------------------------------------
template <typename T>
requires std::floating_point<T>
T multiply(T a, T b) {
    return a * b;
}

// --------------------------------------------------
// 3. Custom Concept – Has .size()
// Checks if object has size() method
// --------------------------------------------------
template <typename T>
concept HasSize = requires(T t) {
    t.size();   // expression must compile
};

template <HasSize T>
void printSize(const T& obj) {
    std::cout << "Size = " << obj.size() << "\n";
}

// --------------------------------------------------
// 4. Custom Concept – Has reserve(size_t)
// Detection idiom via requires
// --------------------------------------------------
template <typename T>
concept Reservable = requires(T t, size_t n) {
    t.reserve(n);
};

template <Reservable T>
void ensureCapacity(T& container, size_t n) {
    container.reserve(n);
    std::cout << "Capacity reserved\n";
}

// --------------------------------------------------
// 5. Multiple Requirements Concept
// Must support + and <
// --------------------------------------------------
template <typename T>
concept AddableComparable = requires(T a, T b) {
    a + b;
    a < b;
};

template <AddableComparable T>
T addIfLess(T a, T b) {
    return (a < b) ? (a + b) : a;
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main() {

    // 1. Integral
    std::cout << add(2, 3) << "\n";          // Works
    // add(2.5, 3.5);                        // Compile error

    // 2. Floating Point
    std::cout << multiply(2.5, 3.0) << "\n";

    // 3. HasSize
    std::vector<int> v{1,2,3};
    std::string s = "hello";
    printSize(v);
    printSize(s);
    // printSize(5);                         // Compile error

    // 4. Reservable
    ensureCapacity(v, 100);
    // ensureCapacity(s, 50);               // string also works actually
    // ensureCapacity(10, 10);              // Compile error

    // 5. AddableComparable
    std::cout << addIfLess(3, 5) << "\n";
    std::cout << addIfLess(7.2, 2.1) << "\n";

    return 0;
}
