#include <mutex>
#include <condition_variable>
#include <deque>

class RWLockStrictFIFO {
    struct Waiter {
        bool writer;
        bool granted = false;
        std::condition_variable cv;
    };

    std::mutex m_;
    std::deque<Waiter*> q_;

    int active_readers_ = 0;
    bool writer_active_ = false;

    // m_ must be held
    void try_grant_front() {
        if (writer_active_ || q_.empty()) return;

        Waiter* w = q_.front();

        if (w->writer) {
            // Head writer can run only when no readers
            if (active_readers_ == 0) {
                writer_active_ = true;
                w->granted = true;
                q_.pop_front();
                w->cv.notify_one();
            }
            return;
        }

        // Head is reader: grant all consecutive readers at the head
        while (!q_.empty() && !q_.front()->writer && !writer_active_) {
            Waiter* r = q_.front();
            r->granted = true;
            ++active_readers_;
            q_.pop_front();
            r->cv.notify_one();
        }
    }

public:
    void lock_read() {
        Waiter me{false};

        std::unique_lock<std::mutex> lk(m_);
        q_.push_back(&me);
        try_grant_front();

        me.cv.wait(lk, [&]{ return me.granted; });
        // When we get here, we’re already counted in active_readers_.
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(m_);
        if (--active_readers_ == 0) {
            try_grant_front(); // may release a head writer (or batch readers)
        }
    }

    void lock_write() {
        Waiter me{true};

        std::unique_lock<std::mutex> lk(m_);
        q_.push_back(&me);
        try_grant_front();

        me.cv.wait(lk, [&]{ return me.granted; });
        // When we get here, writer_active_ is already true.
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(m_);
        writer_active_ = false;
        try_grant_front();
    }
};
