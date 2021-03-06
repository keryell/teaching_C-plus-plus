/** \file

    A Boost.Fiber pool implemented on a std::thread pool

    The use case is for circuit emulation when there are a lot of
    fibers launched at the beginning and they have to run concurrently.
*/

#include <thread>
#include <vector>
#include <boost/fiber/all.hpp>
#include <boost/thread/barrier.hpp>
#include <range/v3/all.hpp>

#include "pooled_shared_work.hpp"
#include "pooled_work_stealing.hpp"

class fiber_pool {

public:

  /// To select some various Boost.Fibers schedulers
  enum class sched {
    round_robin,
    shared_work,
    work_stealing
    // \todo Add numa
  };

private:

  /// The thread running the Boost.Fiber schedulers to do the work
  std::vector<std::future<void>> working_threads;

  /// The queue to submit work
  boost::fibers::unbuffered_channel
  <boost::fibers::packaged_task<void(void)>> submission;

  //static auto constexpr starting_mode = boost::fibers::launch::post;
  static auto constexpr starting_mode = boost::fibers::launch::dispatch;

  /// To synchronize all the threads before they can run some fibers
  boost::barrier starting_block;

  /// To synchronize all the threads after their last fiber
  boost::fibers::barrier finish_line;

  /// To avoid joining several times
  bool joinable = true;

  /// The model of scheduler
  sched s;

  // Pool context for the work-stealing scheduler
  boost::fibers::algo::pooled_work_stealing::ctx pc_stealing;

  // Pool context for the work-sharing scheduler
  boost::fibers::algo::pooled_shared_work::ctx pc_shared;

public:

  /// Create a fiber_pool
  fiber_pool(int thread_number,
             sched scheduler,
             bool suspend)
    : starting_block { static_cast<unsigned int>(thread_number) + 1 }
    , finish_line { static_cast<unsigned int>(thread_number) }
    , s { scheduler }
  {
    if (scheduler == sched::shared_work)
      // This scheduler needs a shared context
      pc_shared = boost::fibers::algo::pooled_shared_work::create_pool_ctx(suspend);
    else if (scheduler == sched::work_stealing)
      // This scheduler needs a shared context
      pc_stealing = boost::fibers::algo::pooled_work_stealing::create_pool_ctx
        (thread_number, suspend);
    // Start the working threads
    working_threads = ranges::iota_view { 0, thread_number }
                    | ranges::views::transform([&] (int i) {
                        return std::async(std::launch::async,
                                          [&, i] { run(i); }); })
                    | ranges::to<std::vector>;
    // Wait for all thread workers to be ready
    starting_block.count_down_and_wait();
  }


  /// Submit some work
  template <typename Callable>
  void submit(Callable && work) {
    submission.push(boost::fibers::packaged_task<void(void)> {
        [f = std::move(work)] { f(); }
          });
  }


  /// Close the submission
  void close() {
    // Can be done many times, so no protection required here
    submission.close();
  }


  /// Wait for all the threads are done
  void join() {
    // Can be done only once
    if (joinable) {
      // Close the submission if not done already
      close();
      for (auto &t : working_threads)
        // A Boost.Fiber scheduler will block its thread if they still
        // have some work to do
        t.get();
      joinable = false;
    }
  }


  /// Wait for some remaining work to be done
  ~fiber_pool() {
    // Join first if not done already
    join();
  }

private:

  /// The thread worker job
  void run(int i) {
    if (s == sched::shared_work)
      boost::fibers::use_scheduling_algorithm
        <boost::fibers::algo::pooled_shared_work>(pc_shared);
    else if (s == sched::work_stealing)
      boost::fibers::use_scheduling_algorithm
        <boost::fibers::algo::pooled_work_stealing>(pc_stealing);
    // Otherwise a round-robin scheduler is used and the fibers will
    // use only 1 thread since there is no thread migration in that
    // case

    // Wait for all thread workers to be ready
    starting_block.count_down_and_wait();

    // Only the first thread receives and starts the work
    if (i == 0) {
      // Keep track of each fiber execution to forward exception if any
      std::vector<boost::fibers::future<void>> futures;
      for (;;) {
        decltype(submission)::value_type work;
        if (submission.pop(work)
            == boost::fibers::channel_op_status::closed)
          // Someone asked to stop accepting work
          break;
        // \todo implement with packaged_task to handle exception and
        // avoid std::function
        futures.push_back(work.get_future());
        // Launch the work on a new unattended fiber
        boost::fibers::fiber { starting_mode, std::move(work) }.detach();
      }
      // Handle any exception here. Well, actually only the first one
      // because it will just throw
      for (auto &f : futures)
        f.get();
    }
    // Wait for all the threads to finish their fiber execution
    finish_line.wait();
  }

};
