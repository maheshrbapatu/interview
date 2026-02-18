// deadlock_avoid_detect.cpp
// Minimal “cheat-sheet” file: deadlock AVOIDANCE + practical DETECTION patterns in C++.
//
// Build: g++ -std=c++20 -O2 -pthread deadlock_avoid_detect.cpp && ./a.out

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ------------------------------------------------------------
// 1) AVOIDANCE: lock multiple mutexes safely in one go
// ------------------------------------------------------------
void avoid_scoped_lock(std::mutex& a, std::mutex& b) {
  // Locks both without deadlock (uses std::lock internally).
  std::scoped_lock lk(a, b);
  // critical section touching both protected resources
}

void avoid_lock_defer(std::mutex& a, std::mutex& b) {
  // Equivalent pattern: defer + std::lock
  std::unique_lock<std::mutex> l1(a, std::defer_lock);
  std::unique_lock<std::mutex> l2(b, std::defer_lock);
  std::lock(l1, l2); // deadlock-avoid multi-lock acquisition
}

// ------------------------------------------------------------
// 2) AVOIDANCE: lock ordering (global order) helper
//    Lock by address (or by id) to avoid cycles.
// ------------------------------------------------------------
struct OrderedLock2 {
  std::unique_lock<std::mutex> l_first;
  std::unique_lock<std::mutex> l_second;

  OrderedLock2(std::mutex& m1, std::mutex& m2)
      : l_first((std::addressof(m1) < std::addressof(m2)) ? m1 : m2),
        l_second((std::addressof(m1) < std::addressof(m2)) ? m2 : m1) {}
};

// ------------------------------------------------------------
// 3) DETECTION-ish: timed locking + backoff/retry
//    Not a proof of deadlock, but catches “stuck” acquisitions.
// ------------------------------------------------------------
struct Backoff {
  int n = 0;
  void operator()() {
    // tiny exponential-ish backoff: spin -> yield -> sleep
    if (n < 10) {
      // spin a little (cheap)
      for (volatile int i = 0; i < (1 << n); ++i) {}
    } else if (n < 20) {
      std::this_thread::yield();
    } else {
      std::this_thread::sleep_for(1ms);
    }
    ++n;
  }
};

bool try_lock_both_with_timeout(std::timed_mutex& a, std::timed_mutex& b,
                               std::chrono::milliseconds budget = 20ms) {
  auto deadline = std::chrono::steady_clock::now() + budget;
  Backoff backoff;

  while (std::chrono::steady_clock::now() < deadline) {
    std::unique_lock<std::timed_mutex> l1(a, std::defer_lock);
    std::unique_lock<std::timed_mutex> l2(b, std::defer_lock);

    // Lock first with tiny bound, then second; if second fails, release + retry.
    if (l1.try_lock_for(1ms)) {
      if (l2.try_lock_for(1ms)) {
        // acquired both
        return true;
      }
    }
    backoff();
  }
  return false; // treat as “possible deadlock or contention storm”
}

// ------------------------------------------------------------
// 4) AVOIDANCE: condition_variable best practice
//    Always wait with a predicate (prevents missed wakeups / spurious wakeups bugs).
// ------------------------------------------------------------
struct QueueGate {
  std::mutex m;
  std::condition_variable cv;
  bool ready = false;

  void producer_set_ready() {
    {
      std::lock_guard<std::mutex> lg(m);
      ready = true;
    }
    cv.notify_one();
  }

  void consumer_wait_ready() {
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul, [&] { return ready; }); // predicate is key
  }
};

// ------------------------------------------------------------
// 5) AVOIDANCE: one-time init without hand-rolled double-checked locking
// ------------------------------------------------------------
std::once_flag g_init_flag;
int* g_resource = nullptr;

void init_resource() {
  // expensive init
  g_resource = new int(123);
}

int get_resource_value() {
  std::call_once(g_init_flag, init_resource);
  return *g_resource;
}

// ------------------------------------------------------------
// 6) DETECTION-ish: task watchdog using future::wait_for
//    Detects “hangs” (deadlock / infinite loop / blocking IO).
//    Limitation: standard C++ cannot safely kill the stuck thread.
// ------------------------------------------------------------
template <class F>
void run_with_timeout(F&& f, std::chrono::milliseconds timeout) {
  auto fut = std::async(std::launch::async, std::forward<F>(f));
  if (fut.wait_for(timeout) != std::future_status::ready) {
    throw std::runtime_error("Timeout: possible deadlock/hang detected");
  }
  fut.get();
}

// ------------------------------------------------------------
// Demo main: shows patterns compile/run (not a full test suite).
// ------------------------------------------------------------
int main() {
  std::cout << "Deadlock avoidance/detection mini-demo\n";

  // --- scoped_lock / std::lock pattern ---
  std::mutex m1, m2;
  avoid_scoped_lock(m1, m2);
  avoid_lock_defer(m1, m2);

  // --- lock ordering helper ---
  {
    OrderedLock2 lk(m1, m2);
    (void)lk;
  }

  // --- timed try-lock w/ backoff ---
  std::timed_mutex tm1, tm2;
  bool ok = try_lock_both_with_timeout(tm1, tm2, 30ms);
  std::cout << "try_lock_both_with_timeout acquired both? " << (ok ? "yes" : "no") << "\n";

  // --- condition_variable predicate pattern ---
  QueueGate gate;
  std::thread t([&] {
    gate.consumer_wait_ready();
    std::cout << "consumer: ready observed\n";
  });
  std::this_thread::sleep_for(5ms);
  gate.producer_set_ready();
  t.join();

  // --- call_once ---
  std::cout << "call_once resource value: " << get_resource_value() << "\n";

  // --- watchdog timeout around potentially stuck task ---
  try {
    run_with_timeout([] {
      // simulate "stuck" by sleeping too long
      std::this_thread::sleep_for(200ms);
    }, 50ms);
  } catch (const std::exception& e) {
    std::cout << "watchdog caught: " << e.what() << "\n";
  }

  std::cout << "Done.\n";
  return 0;
}
