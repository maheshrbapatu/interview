class Car {
public:
    int wheels = 0;
};

class CarBuilder {
    Car c;
public:
    CarBuilder& setWheels(int w) { c.wheels = w; return *this; }
    Car build() { return c; }
};
`
