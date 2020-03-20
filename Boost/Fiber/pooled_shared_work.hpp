//          Copyright Nat Goodspeed + Oliver Kowalke 2015
//                                  + Ronan Keryell 2020
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_ALGO_POOLED_SHARED_WORK_H
#define BOOST_FIBERS_ALGO_POOLED_SHARED_WORK_H

#include <condition_variable>
#include <chrono>
#include <deque>
#include <mutex>

#include <boost/config.hpp>
#include <boost/assert.hpp>

#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/scheduler.hpp>
#include "boost/fiber/type.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif

namespace boost::fibers::algo {

class BOOST_FIBERS_DECL pooled_shared_work : public algorithm {

  using rqueue_type = std::deque<context *>;
  using lqueue_type = scheduler::ready_queue_type;

 public:

  /// Shared storage among the working threads
  struct pool_ctx {
    pool_ctx(bool suspend) : suspend_ { suspend }
    {}

    /// Indicate if a thread without work goes to sleep instead of busy-waiting
    const bool suspend_;

    /// The global queue storing the runnable fibers
    rqueue_type rqueue_ {};

    /// For concurrent access to the global queue
    std::mutex rqueue_mtx_ {};
  };

  /// Type tracking the common worker data
  using ctx = std::shared_ptr<pool_ctx>;

 private:

  /// Some shared datastructure among the working threads
  ctx pool_ctx_;

  /// The runnable local fibers bound to the thread
  lqueue_type lqueue_ {};

  /// The thread-local suspend/notify mechanics
  std::mutex mtx_ {};
  std::condition_variable cnd_ {};
  bool flag_ { false };

public:

  static ctx
  create_pool_ctx(bool suspend) {
    return std::make_shared<pool_ctx>(suspend);
  }


  pooled_shared_work(const ctx &pc) : pool_ctx_ { pc }
  {}

  pooled_shared_work(pooled_shared_work const&) = delete;
  pooled_shared_work(pooled_shared_work &&) = delete;

  pooled_shared_work & operator=(pooled_shared_work const&) = delete;
  pooled_shared_work & operator=(pooled_shared_work &&) = delete;

  void awakened(context * ctx) noexcept override {
    if (ctx->is_context(type::pinned_context)) { /*<
            recognize when we're passed this thread's main fiber (or an
            implicit library helper fiber): never put those on the shared
            queue
      >*/
      lqueue_.push_back(*ctx);
    } else {
      ctx->detach();
      std::unique_lock lk { pool_ctx_->rqueue_mtx_ }; /*<
            worker fiber, enqueue on shared queue
      >*/
      pool_ctx_->rqueue_.push_back(ctx);
    }
  }


  context * pick_next() noexcept override {
    context * ctx = nullptr;
    std::unique_lock lk { pool_ctx_->rqueue_mtx_ };
    auto &rq = pool_ctx_->rqueue_;
    if (!rq.empty()) { /*<
            pop an item from the ready queue
      >*/
      ctx = rq.front();
      rq.pop_front();
      lk.unlock();
      BOOST_ASSERT(nullptr != ctx);
      context::active()->attach(ctx); /*<
            attach context to current scheduler via the active fiber
            of this thread
       >*/
    } else {
      lk.unlock();
      if (!lqueue_.empty()) { /*<
                nothing in the ready queue, return main or dispatcher fiber
        >*/
        ctx = & lqueue_.front();
        lqueue_.pop_front();
      }
    }
    return ctx;
  }


  bool has_ready_fibers() const noexcept override {
    std::unique_lock lock { pool_ctx_->rqueue_mtx_ };
    return !pool_ctx_->rqueue_.empty() || !lqueue_.empty();
  }


  void suspend_until(std::chrono::steady_clock::time_point const& time_point)
    noexcept override {
    if (pool_ctx_->suspend_) {
      if (std::chrono::steady_clock::time_point::max() == time_point) {
        std::unique_lock lk { mtx_ };
        cnd_.wait(lk, [this](){ return flag_; });
        flag_ = false;
      } else {
        std::unique_lock lk { mtx_ };
        cnd_.wait_until(lk, time_point, [this](){ return flag_; });
        flag_ = false;
      }
    }
  }


  void notify() noexcept override {
    if (pool_ctx_->suspend_) {
      std::unique_lock lk { mtx_ };
      flag_ = true;
      lk.unlock();
      cnd_.notify_all();
    }
  }

};

}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_ALGO_POOLED_SHARED_WORK_H
