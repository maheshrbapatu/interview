#include <iostream>
using namespace std;

/*
C++17 Fold Expressions

4 forms:
1. Unary Left   : (... op pack)
2. Unary Right  : (pack op ...)
3. Binary Left  : (init op ... op pack)
4. Binary Right : (pack op ... op init)
*/

// ---------- 1. UNARY LEFT FOLD ----------
template<typename... Ts>
auto sum_unary_left(Ts... xs) {
    // Expands like: (((x1 + x2) + x3) + ... )
    return (... + xs);
}

// ---------- 2. UNARY RIGHT FOLD ----------
template<typename... Ts>
auto sum_unary_right(Ts... xs) {
    // Expands like: (x1 + (x2 + (x3 + ... )))
    return (xs + ...);
}

// ---------- 3. BINARY LEFT FOLD ----------
template<typename... Ts>
auto sum_binary_left(Ts... xs) {
    // Init value = 0
    // Expands like: (((0 + x1) + x2) + x3) ...
    return (0 + ... + xs);
}

// ---------- 4. BINARY RIGHT FOLD ----------
template<typename... Ts>
auto sum_binary_right(Ts... xs) {
    // Init value = 0
    // Expands like: (x1 + (x2 + (x3 + 0)))
    return (xs + ... + 0);
}

// PRINT EXAMPLE (binary left fold with cout)
template<typename... Ts>
void print(Ts... args) {
    // (((cout << a1) << a2) << a3) ...
    (cout << ... << args);
    cout << "\n";
}

int main() {

    cout << "Unary Left:  " << sum_unary_left(1,2,3) << "\n";
    cout << "Unary Right: " << sum_unary_right(1,2,3) << "\n";
    cout << "Binary Left: " << sum_binary_left(1,2,3) << "\n";
    cout << "Binary Right:" << sum_binary_right(1,2,3) << "\n";

    print("x=", 10, " y=", 2.5);

    return 0;
}
