class OldAPI {
public:
    void oldFunc() {}
};

class Adapter {
    OldAPI api;
public:
    void newFunc() { api.oldFunc(); }
};

