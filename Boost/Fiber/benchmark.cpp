/** \file

    Estimate the Boost Fiber context switch duration

    The use case is for circuit emulation when there are a lot of
    fibers launched at the beginning and they have to run concurrently.

    We are *not* interested in the launch/join behaviour exposed for
    example in https://github.com/atemerev/skynet since it is assumed
    to be amortize on the global long running time.
*/

#include <atomic>
#include <chrono>
#include <iostream>

#include "fiber_pool.hpp"
#ifdef TRISYCL_FIBER_POOL_DEBUG
#define TRISYCL_FIBER_POOL_DEBUG_EXEC(...) __VA_ARGS__
#else
#endif


// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

/// A parametric benchmark
void benchmark(int thread_number,
               int fiber_number,
               int iterations,
               fiber_pool::sched scheduler,
               bool suspend) {
  std::cout << "threads: " << thread_number
            << " fibers: "<< fiber_number
            << " iterations: " << iterations
            << " scheduler: " << static_cast<int>(scheduler)
            << " suspend: " << static_cast<int>(suspend) << std::endl;

  fiber_pool fp { thread_number, scheduler, suspend };

#ifdef TRISYCL_FIBER_POOL_DEBUG
  std::atomic<std::int64_t> c = 0;
  std::atomic<std::int64_t> f = 0;
  std::atomic<std::int64_t> s = 0;
#endif

  /// The basic benchmark is fiber doing a lot of yield()
  auto bench = [&] {
 #ifdef TRISYCL_FIBER_POOL_DEBUG
                ++s;
#endif
                 for (auto counter = iterations; counter != 0; --counter) {
                   boost::this_fiber::yield();
#ifdef TRISYCL_FIBER_POOL_DEBUG
                   ++c;
#endif
                 }
#ifdef TRISYCL_FIBER_POOL_DEBUG
                 ++f;
#endif
               };

  auto starting_point = clk::now();

  // The first thread start fiber_number fibers running bench
  for (int i = fiber_number; i != 0; --i)
    fp.submit(bench);

  fp.join();

  // Get the duration in seconds as a double
  std::chrono::duration<double> duration = clk::now() - starting_point;

  // In s
  std::cout << " time: " << duration.count()
            << " inter context switch: "
               // In ns
            << duration.count()/iterations/fiber_number*1e9 << std::endl;
#ifdef TRISYCL_FIBER_POOL_DEBUG
  std::cout << " S: " << s << " F: " << f << " C: " << c << std::endl;
  assert(s == fiber_number
         && "we should have the same number of start as the number of fibers");
  assert(s == f && "we should have the same number of start and finish");
  assert(c == fiber_number*iterations
         && "we should have the right number of global interations");
#endif
}

int main() {
  for (std::size_t thread_number = 1;
       thread_number <= 2*std::thread::hardware_concurrency();
       ++thread_number)
    for (auto fiber_number : { 1, 3, 10, 30, 100, 300, 1000, 3000 })
      for (auto iterations : { 1e4, 1e5, 1e6 })
        for (auto scheduler : { fiber_pool::sched::round_robin,
                                fiber_pool::sched::shared_work,
                                fiber_pool::sched::work_stealing }) {
          benchmark(thread_number,
                    fiber_number,
                    iterations,
                    scheduler,
                    false);
          if (scheduler != fiber_pool::sched::round_robin)
            // The same but with the thread suspension when no work
            benchmark(thread_number,
                      fiber_number,
                      iterations,
                      scheduler,
                      true);
        }
}
