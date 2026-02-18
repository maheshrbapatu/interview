#include <iostream>
#include <memory>
using namespace std;

int main() {
    weak_ptr<int> wp;

    {
        auto sp = make_shared<int>(10); // object created
        wp = sp;                        // weak observes

        if (auto locked = wp.lock()) {  // try to get shared_ptr
            cout << "Value: " << *locked << "\n";
        }
    } // sp destroyed → object gone

    if (auto locked = wp.lock()) {
        cout << "Still alive\n";
    } else {
        cout << "Object gone\n";
    }
}
