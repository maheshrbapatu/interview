#include <iostream>
#include <string>

template <typename T>
struct Printer {
    static void print(const T& x) { std::cout << x << "\n"; }
};

template <>
struct Printer<std::string> {
    static void print(const std::string& s) { std::cout << '"' << s << "\"\n"; }
};

int main() {
    Printer<int>::print(10);
    Printer<std::string>::print("hi");
}

