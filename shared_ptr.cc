#include <iostream>

struct control_block {
    int ref_count;
    explicit control_block(int c) : ref_count(c) {}
};

template <typename T>
class shared_ptr {
    T* ptr = nullptr;
    control_block* cb = nullptr;

    void dec_ref_and_cleanup() {
        if (!cb) return;
        cb->ref_count--;
        if (cb->ref_count == 0) {
            delete ptr;
            delete cb;
        }
        ptr = nullptr;
        cb = nullptr;
    }

public:
    shared_ptr() = default;

    explicit shared_ptr(T* p) : ptr(p) {
        if (p) cb = new control_block(1);
    }

    ~shared_ptr() { dec_ref_and_cleanup(); }

    shared_ptr(const shared_ptr& other) : ptr(other.ptr), cb(other.cb) {
        if (cb) cb->ref_count++;
    }

    shared_ptr(shared_ptr&& other) noexcept : ptr(other.ptr), cb(other.cb) {
        other.ptr = nullptr;
        other.cb = nullptr;
    }

    shared_ptr& operator=(const shared_ptr& other) {
        if (this == &other) return *this;
        dec_ref_and_cleanup();
        ptr = other.ptr;
        cb = other.cb;
        if (cb) cb->ref_count++;
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        if (this == &other) return *this;
        dec_ref_and_cleanup();
        ptr = other.ptr;
        cb = other.cb;
        other.ptr = nullptr;
        other.cb = nullptr;
        return *this;
    }

    T* get() const { return ptr; }
    int use_count() const { return cb ? cb->ref_count : 0; }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    void reset(T* p = nullptr) {
        dec_ref_and_cleanup();
        ptr = p;
        cb = p ? new control_block(1) : nullptr;
    }
};

