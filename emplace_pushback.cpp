//
// Created by Mahesh Bapatu on 10/12/23.
//
#include <vector>
class MyClass {
public:
    MyClass(int arg1, double arg2) {
        // Constructor implementation
    }
    // Other members of MyClass...
};

int main() {
    std::vector<MyClass> my_vector;

    // Constructs an object of MyClass in-place with the arguments 42 and 3.14
    // Constructs an element in-place at the end of the container.
    // It uses arguments that are passed to the constructor of the element type and
    // directly constructs the object within the space managed by the container.
    // This can be more efficient because it eliminates the need for a temporary object
    // and can reduce the number of copies or moves required
    my_vector.emplace_back(42, 3.14);

    // The above is equivalent to the following, but without creating a temporary:
    // push_back: Inserts a copy (or moves) of the element at the end of the container.
    // If you use push_back, the object is fully constructed and then copied or moved
    // into the container (which could involve additional overhead, especially for complex objects).
     MyClass temp(42, 3.14);
     my_vector.push_back(temp);

    // Or, if MyClass is not copyable but movable:
     my_vector.push_back(MyClass(42, 3.14));
    return 0;
}