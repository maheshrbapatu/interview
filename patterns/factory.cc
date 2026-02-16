#include <memory>

struct Shape {
    virtual void draw() = 0;
    virtual ~Shape() = default;
};

struct Circle : Shape {
    void draw() override { std::cout << "Circle\n"; }
};

struct Square : Shape {
    void draw() override { std::cout << "Square\n"; }
};

std::unique_ptr<Shape> makeShape(int type) {
    if (type == 1) return std::make_unique<Circle>();
    return std::make_unique<Square>();
}
`
