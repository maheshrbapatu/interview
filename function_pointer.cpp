#include <iostream>
#include <functional>
using namespace std;

class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int mul(int a, int b) const { return a * b; }
};

/************ (2) Template helper: generic caller for member function pointers ************/
template <typename T, typename Ret, typename... Args>
Ret call_member(const T& obj, Ret (T::*mfp)(Args...) const, Args... args) {
    return (obj.*mfp)(args...);
}

int main() {
    Calculator c;

    /************ (1) Member function pointer ************/
    int (Calculator::*mfp_add)(int, int) const = &Calculator::add;
    cout << "member function pointer: " << (c.*mfp_add)(3, 4) << "\n";

    /************ (2) Member function pointer used via templates ************/
    cout << "template caller:         " << call_member(c, &Calculator::mul, 3, 4) << "\n";

    /************ (3) std::function wrapping a class function (via lambda capture) ************/
    std::function<int(int,int)> sf = [&c](int a, int b) {
        return c.add(a, b);
    };
    cout << "std::function:           " << sf(10, 20) << "\n";

    /************ (4) std::bind binding object + member function ************/
    auto bound = std::bind(&Calculator::mul, &c,
                           std::placeholders::_1,
                           std::placeholders::_2);
    cout << "std::bind:               " << bound(6, 7) << "\n";

    return 0;
}
