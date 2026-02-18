#include <iostream>
#include <memory>
using namespace std;

int main() {
    weak_ptr<int> wp;

    {
        auto sp = make_shared<int>(42);
        wp = sp;   // weak_ptr observes, does not own

        if (auto locked = wp.lock()) {
            cout << "Value: " << *locked << "\n";
        }
    } // sp goes out of scope → object destroyed

    if (auto locked = wp.lock()) {
        cout << "Still alive\n";
    } else {
        cout << "Object gone\n";
    }
}
