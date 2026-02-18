#include <iostream>
#include <functional>
using namespace std;

class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int mul(int a, int b) const { return a * b; }
};

/******** TEMPLATE GENERIC CALLER USING std::invoke ********/
template <typename Callable, typename... Args>
decltype(auto) call_any(Callable&& fn, Args&&... args) {
    // invoke is needed here because "fn" could be:
    //  - a member function pointer (needs special call syntax)
    //  - a normal function/lambda/functor (callable via operator())
    return std::invoke(std::forward<Callable>(fn),
                       std::forward<Args>(args)...);
}

int main() {
    Calculator c;

    /******** 1) MEMBER FUNCTION POINTER ********/
    
    // return_type (*pointer_name)(parameter_types);
    
    // invoke IS needed here (member function pointers can't be called like a normal function)
    auto mfp = &Calculator::add;
    cout << "member ptr + invoke: " << std::invoke(mfp, c, 3, 4) << "\n";

    /******** 2) TEMPLATE (generic) ********/
    // invoke IS needed inside call_any for generic callability
    cout << "template invoke:     " << call_any(&Calculator::mul, c, 5, 6) << "\n";

    /******** 3) std::function ********/
    // invoke is NOT needed here; std::function is directly callable
    std::function<int(int,int)> sf = [&c](int a, int b) { return c.add(a, b); };
    cout << "std::function:       " << sf(7, 8) << "\n";

    /******** 4) std::bind ********/
    // bind is ONLY "needed" if you want to bind the object now and call later like a free function
    auto bound = std::bind(&Calculator::mul, &c,
                           std::placeholders::_1,
                           std::placeholders::_2);

    // invoke is NOT needed here either; bind returns a callable object
    cout << "bind (callable):     " << bound(9, 10) << "\n";
}
