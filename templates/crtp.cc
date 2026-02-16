#include <iostream>

template <class D>
struct Shape {
    void draw() {                 // common API
        static_cast<D*>(this)->draw_impl();   // derived hook
    }
};

struct Circle : Shape<Circle> {
    void draw_impl() { std::cout << "Circle\n"; }
};

struct Square : Shape<Square> {
    void draw_impl() { std::cout << "Square\n"; }
};

int main() {
    Circle c; Square s;
    c.draw();
    s.draw();
}

