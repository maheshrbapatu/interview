struct Coffee {
    virtual int cost() { return 5; }
};

struct Milk : Coffee {
    Coffee& base;
    Milk(Coffee& c) : base(c) {}
    int cost() override { return base.cost() + 2; }
};
