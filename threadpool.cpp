#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

class SimpleThreadPool {
public:
    explicit SimpleThreadPool(size_t n) {
        if (n == 0) n = 1;
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this, i] { workerLoop(i); });
        }
    }

    SimpleThreadPool(const SimpleThreadPool&) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;

    // Fire-and-forget submit
    void submit(std::function<void()> job) {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stopping_) return;            // or throw; your choice
            q_.push(std::move(job));
        }
        cv_.notify_one();
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(m_);
            stopping_ = true;
        }
        cv_.notify_all();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
        workers_.clear();
    }

    ~SimpleThreadPool() {
        shutdown();
    }

private:
    void workerLoop(size_t workerId) {
        while (true) {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lk(m_);
                cv_.wait(lk, [&] { return stopping_ || !q_.empty(); });

                if (stopping_ && q_.empty()) return;

                job = std::move(q_.front());
                q_.pop();
            }

            // Run outside lock
            job();
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex m_;
    std::condition_variable cv_;
    bool stopping_ = false;
};

// ---------------- Demo ----------------
int main() {
    SimpleThreadPool pool(3);

    for (int i = 1; i <= 8; ++i) {
        pool.submit([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            std::cout << "Task " << i << " done on thread " 
                      << std::this_thread::get_id() << "\n";
        });
    }

    // Give tasks time to run (since we aren't waiting on futures)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // pool.shutdown(); // optional (destructor does it)
    return 0;
}
