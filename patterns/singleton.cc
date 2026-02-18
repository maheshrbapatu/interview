class Singleton {
private:
    Singleton() = default;
    ~Singleton() = default;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

public:
    static Singleton& instance() {
        static Singleton obj;   // created once, lazy, thread-safe (C++11+)
        return obj;
    }

    void foo() {
        // ...
    }
};

int main() {
    Singleton::instance().foo();
}
