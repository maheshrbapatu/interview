#include <iostream>

class Logger {
public:
    static Logger& instance() {
        static Logger inst;   // thread-safe since C++11
        return inst;
    }

    void log(const std::string& s) { std::cout << s << "\n"; }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

int main() {
    Logger::instance().log("hello");
}

