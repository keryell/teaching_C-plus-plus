/** \file

    Estimate the Boost Fiber context switch duration

    The use case is for circuit emulation when there are a lot of
    fibers launched at the beginning and they have to run concurrently.

    We are *not* interested in the launch/join behaviour exposed for
    example in https://github.com/atemerev/skynet since it is assumed
    to be amortize on the global long running time.
*/

#include <chrono>
#include <iostream>
#include <vector>
#include <boost/fiber/all.hpp>
#include <boost/thread/barrier.hpp>
#include <range/v3/all.hpp>

/// To select some various Boost.Fibers schedulers
enum class sched {
  round_robin,
  shared_work,
  work_stealing
};

//auto constexpr starting_mode = boost::fibers::launch::dispatch;
auto constexpr starting_mode = boost::fibers::launch::post;

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

/// Install the requested scheduler in the thread executing this function
void install_fiber_scheduler(sched scheduler, int thread_number, bool suspend) {
  if (scheduler == sched::shared_work)
    boost::fibers::use_scheduling_algorithm
      <boost::fibers::algo::shared_work>();
  if (scheduler == sched::work_stealing)
    boost::fibers::use_scheduling_algorithm
      <boost::fibers::algo::work_stealing>(thread_number, suspend);
}

/// A parametric benchmark
void bench_mark(int thread_number,
                int fiber_number,
                int iterations,
                sched scheduler,
                bool suspend) {
  std::cout << "threads: " << thread_number
            << " fibers: "<< fiber_number
            << " iterations: " << iterations
            << " scheduler: " << static_cast<int>(scheduler)
            << " suspend: " << static_cast<int>(suspend) << std::flush;

  /// The basic benchmark is fiber doing a lot of yield()
  auto bench = [&] {
                 for (auto counter = iterations; counter != 0; --counter)
                   boost::this_fiber::yield();
               };

  // Install fiber scheduler in main thread
  install_fiber_scheduler(scheduler, thread_number, suspend);

  /// To synchronize all the threads before they can run some fibers
  boost::barrier b { static_cast<unsigned int>(thread_number) };

  // Just to block the working threads while there is some work to do
  boost::fibers::unbuffered_channel<int> blocker;

  // Start more threads, beside the main one
  auto threads = ranges::iota_view { 0, thread_number - 1}
  | ranges::views::transform([&] (auto i) {
      return std::async(std::launch::async,
                        [&] {
                          install_fiber_scheduler(scheduler,
                                                  thread_number,
                                                  suspend);
                          // Wait for other threads to be ready
                          b.count_down_and_wait();
                          try {
                            (void) blocker.value_pop();
                          } catch(...) {}
                        }); })
  | ranges::to<std::vector>;

  // Wait for all the fiber schedulers to be installed in each thread
  // before launching any fiber, otherwise it crashes
  b.count_down_and_wait();

  auto starting_point = clk::now();

  // Start fiber_number entities running bench
  auto fibers = ranges::iota_view { 0, fiber_number }
  | ranges::views::transform([&] (auto i) {
      return boost::fibers::fiber { starting_mode, bench }; })
  | ranges::to<std::vector>;

  // Wait for everybody to finish
  for (auto &f : fibers)
    f.join();

  // Get the duration in seconds as a double
  std::chrono::duration<double> duration = clk::now() - starting_point;
  // In s
  std::cout << " time: " << duration.count()
            <<" inter context switch: "
    // In ns
            << duration.count()/iterations/fiber_number*1e9 << std::endl;

  // Unleash the threads
  blocker.close();

  for (auto &t : threads)
    t.get();
}

int main() {
  for (int thread_number = 1; thread_number <= 20; ++thread_number)
    for (auto fiber_number : { 1, 3, 10, 30, 100, 300, 1000, 3000 })
      for (auto iterations : { 1e4, 1e5, 1e6 })
        for (auto scheduler :
             { sched::round_robin, sched::shared_work, sched::work_stealing}) {
          bench_mark(thread_number,
                     fiber_number,
                     iterations,
                     scheduler,
                     false);
          if (scheduler == sched::work_stealing)
            bench_mark(thread_number,
                       fiber_number,
                       iterations,
                       scheduler,
                       true);
        }
}
