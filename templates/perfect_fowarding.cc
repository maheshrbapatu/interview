/*
PERFECT FORWARDING – SINGLE SELF-CONTAINED DEMO

Goal:
Write a wrapper that forwards arguments to another function
WITHOUT changing:
- lvalue vs rvalue
- constness
- reference category
- move semantics

Key tools:
1. T&& in a template parameter  -> "forwarding reference"
2. std::forward<T>(x)           -> preserves original value category
*/

   /*
    PERFECT FORWARDING – LVALUE CASE EXPLANATION

    Suppose caller does:
        int a = 10;
        good_wrapper(a);

    1. Template Deduction:
       'a' is an LVALUE int
       So compiler deduces:
           T = int&

    2. Parameter Type Formation:
       Function parameter is T&&
       Substitute T = int&:
           T&&  →  int& &&
       Reference collapsing rule:
           & + &&  →  &
       So parameter becomes:
           int& x   (still an lvalue reference)

    3. std::forward<T>(x):
       Here T = int&
       std::forward<int&>(x) returns int&
       (NOT an rvalue, no move)

    Result:
       The original lvalue nature is preserved.
       If process(int&) exists, that overload is called.

    Key Idea:
       Caller lvalue → T deduced as reference → std::forward restores lvalue.
    */

#include <iostream>
#include <utility>
using namespace std;

// ---------- TARGET FUNCTIONS (to observe behavior) ----------

// Overload for lvalue
void process(int& x) {
    cout << "process(lvalue)\n";
}

// Overload for rvalue
void process(int&& x) {
    cout << "process(rvalue)\n";
}

// ---------- WRONG WRAPPER (NO PERFECT FORWARDING) ----------
template<typename T>
void bad_wrapper(T x) {
    // x is always an lvalue inside this function
    // even if caller passed rvalue
    process(x);  // rvalues get downgraded to lvalues
}

// ---------- CORRECT WRAPPER (PERFECT FORWARDING) ----------
template<typename T>
void good_wrapper(T&& x) {
    // T&& here is a *forwarding reference*
    // std::forward<T>(x) preserves lvalue/rvalue nature
    process(std::forward<T>(x));
}

// ---------- GENERIC FORWARDER FOR ANY FUNCTION ----------
template<typename F, typename... Args>
decltype(auto) forwarder(F&& f, Args&&... args) {
    // forward callable + arguments
    return std::forward<F>(f)(
        std::forward<Args>(args)...
    );
}

int main() {

    int a = 10;

    cout << "---- Bad Wrapper ----\n";
    bad_wrapper(a);   // lvalue -> lvalue
    bad_wrapper(20);  // rvalue -> STILL lvalue (wrong!)

    cout << "\n---- Good Wrapper ----\n";
    good_wrapper(a);   // lvalue preserved
    good_wrapper(20);  // rvalue preserved

    cout << "\n---- Generic Forwarder ----\n";
    forwarder(process, a);   // lvalue
    forwarder(process, 30);  // rvalue

    return 0;
}
