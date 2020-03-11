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

auto constexpr iterations = 1e6;
auto constexpr fiber_number = 400;
auto constexpr thread_number = 6;

//auto constexpr starting_mode = boost::fibers::launch::dispatch;
auto constexpr starting_mode = boost::fibers::launch::post;
//auto constexpr scheduler = sched::round_robin;
//auto constexpr scheduler = sched::shared_work;
auto constexpr scheduler = sched::work_stealing;
auto constexpr suspend = false;
//auto constexpr suspend = true;

using clk = std::chrono::high_resolution_clock;

/// To synchronize all the threads before they can run some fibers
boost::barrier b { thread_number };

/// Install the requested scheduler in the thread executing this function
void install_fiber_scheduler() {
  if constexpr (scheduler == sched::shared_work)
    boost::fibers::use_scheduling_algorithm
      <boost::fibers::algo::shared_work>();
  if constexpr (scheduler == sched::work_stealing)
    boost::fibers::use_scheduling_algorithm
      <boost::fibers::algo::work_stealing>(thread_number, suspend);
}


int main() {
  /// The basic benchmark is fiber doing a lot of yield()
  auto bench = [] {
    for (auto counter = iterations; counter != 0; --counter)
      boost::this_fiber::yield();
  };

  // Install fiber scheduler in main thread
  install_fiber_scheduler();

  // Just to block the working threads while there is some work to do
  boost::fibers::unbuffered_channel<int> blocker;

  // Start more threads, beside the main one
  auto threads = ranges::iota_view { 0, thread_number - 1}
  | ranges::views::transform([&] (auto i) {
      return std::async(std::launch::async,
                        [&] {
                          install_fiber_scheduler();
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
  std::cout << "Execution time = " << duration.count() << " s" << std::endl;
  std::cout << "Time between context switches = "
            << duration.count()/iterations/fiber_number*1e9
            << " ns" << std::endl;

  // Unleash the threads
  blocker.close();

  for (auto &t : threads)
    t.get();

}
