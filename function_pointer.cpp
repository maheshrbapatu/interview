#include <iostream>
#include <functional>
using namespace std;

class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int mul(int a, int b) const { return a * b; }
};

/******** TEMPLATE GENERIC CALLER USING std::invoke ********/
template <typename Callable, typename Obj, typename... Args>
auto call_any(Callable&& fn, Obj&& obj, Args&&... args) {
    return std::invoke(std::forward<Callable>(fn),
                       std::forward<Obj>(obj),
                       std::forward<Args>(args)...);
}

int main() {
    Calculator c;

    /******** 1. MEMBER FUNCTION POINTER ********/
    auto mfp = &Calculator::add;
    cout << "member ptr + invoke: "
         << std::invoke(mfp, c, 3, 4) << "\n";

    /******** 2. TEMPLATE + INVOKE ********/
    cout << "template invoke:    "
         << call_any(&Calculator::mul, c, 5, 6) << "\n";

    /******** 3. std::function ********/
    std::function<int(int,int)> sf =
        [&c](int a, int b) { return c.add(a, b); };

    cout << "std::function:      "
         << std::invoke(sf, 7, 8) << "\n";

    /******** 4. std::bind ********/
    auto bound = std::bind(&Calculator::mul, &c,
                           std::placeholders::_1,
                           std::placeholders::_2);

    cout << "bind + invoke:      "
         << std::invoke(bound, 9, 10) << "\n";
}
