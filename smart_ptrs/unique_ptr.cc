#include <iostream>

template <typename T>
class unique_ptr {
    T* ptr = nullptr;

public:
    // default
    unique_ptr() = default;

    // ctor from raw pointer
    explicit unique_ptr(T* p) : ptr(p) {}

    // destructor
    ~unique_ptr() {
        delete ptr;
    }

    // NO COPY
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // MOVE ctor
    unique_ptr(unique_ptr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    // MOVE assignment
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this == &other) return *this;

        delete ptr;              // clean current
        ptr = other.ptr;         // steal
        other.ptr = nullptr;     // empty source
        return *this;
    }

    // observers
    T* get() const { return ptr; }
    explicit operator bool() const { return ptr != nullptr; }

    // dereference
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    // modifiers
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;             // caller must delete
    }

    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
};

