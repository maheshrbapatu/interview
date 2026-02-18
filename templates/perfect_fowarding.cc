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
