#include <iostream>
#include <string>

template <typename T>
struct Printer {
    static void print(const T& x) { std::cout << x << "\n"; }
};

// full specialization
template <>
struct Printer<std::string> {
    static void print(const std::string& s) { std::cout << '"' << s << "\"\n"; }
};


// Primary template (generic)
template<typename T>
struct Box {
    static constexpr const char* tag = "primary: Box<T>";
};

// Partial specialization for pointers
template<typename T>
struct Box<T*> {
    static constexpr const char* tag = "partial: Box<T*>";
};

// Partial specialization for const types
template<typename T>
struct Box<const T> {
    static constexpr const char* tag = "partial: Box<const T>";
};

int main() {
    Printer<int>::print(10);
    Printer<std::string>::print("hi");
}

