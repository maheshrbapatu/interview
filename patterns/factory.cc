#include <iostream>
#include <memory>

struct Shape {
    virtual void draw() = 0;
    virtual ~Shape() = default;   // always virtual for polymorphic base
};

struct Circle : Shape {
    void draw() override {
        std::cout << "Circle\n";
    }
};

struct Square : Shape {
    void draw() override {
        std::cout << "Square\n";
    }
};

// Factory function
std::unique_ptr<Shape> makeShape(int type) {
    if (type == 1)
        return std::make_unique<Circle>();
    return std::make_unique<Square>();
}

int main() {
    // create via factory
    std::unique_ptr<Shape> s1 = makeShape(1);
    std::unique_ptr<Shape> s2 = makeShape(2);

    // runtime polymorphism
    s1->draw();
    s2->draw();

    // also works inline
    makeShape(1)->draw();

    return 0;
}
