#include <iostream>
using namespace std;

// Normal function
int add(int a, int b) {
    return a + b;
}

int main() {
    // Function pointer declaration
    int (*fp)(int, int);

    // Assign function address to pointer
    fp = add;

    // Call using normal function
    int r1 = add(2, 3);

    // Call using function pointer
    int r2 = fp(2, 3);

    cout << "Normal call: " << r1 << "\n";
    cout << "Pointer call: " << r2 << "\n";
}
