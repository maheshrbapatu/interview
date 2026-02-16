#include <iostream>
#include <type_traits>

template <typename T>
struct is_pointer_trait : std::false_type {};

template <typename T>
struct is_pointer_trait<T*> : std::true_type {};

template <typename T>
void print_kind() {
    if constexpr (is_pointer_trait<T>::value) std::cout << "pointer\n";
    else std::cout << "not pointer\n";
}

int main() {
    print_kind<int>();
    print_kind<int*>();
}

