#include <vector>
#include <functional>

class Subject {
    std::vector<std::function<void(int)>> observers;
public:
    void subscribe(std::function<void(int)> fn) {
        observers.push_back(fn);
    }

    void notify(int value) {
        for (auto& fn : observers) fn(value);
    }
};

int main() {
    Subject s;
    s.subscribe([](int x){ std::cout << "A got " << x << "\n"; });
    s.subscribe([](int x){ std::cout << "B got " << x << "\n"; });

    s.notify(42);
}

