#include <iostream>

template <typename... Args>
void log_all(const Args&... args) {
    ((std::cout << args << ' '), ...);
    std::cout << '\n';
}

int main() {
    log_all("x=", 10, "y=", 2.5);
}

