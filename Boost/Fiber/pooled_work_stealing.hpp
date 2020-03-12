// This file is adapted from
// https://github.com/boostorg/fiber/blob/develop/include/boost/fiber/algo/work_stealing.hpp
//
//          Copyright Oliver Kowalke 2015 / Lennart Braun 2019
//          / Ronan Keryell 2020

//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_FIBERS_ALGO_POOLED_WORK_STEALING_H
#define BOOST_FIBERS_ALGO_POOLED_WORK_STEALING_H

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#include <boost/config.hpp>
#include <boost/context/detail/prefetch.hpp>
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/context_spinlock_queue.hpp>
#include <boost/fiber/detail/context_spmc_queue.hpp>
#include <boost/fiber/scheduler.hpp>
#include <boost/thread/barrier.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif
namespace boost::fibers::algo {

class pooled_work_stealing;

struct pool_ctx {
  pool_ctx( std::uint32_t thread_count, bool suspend)
    : thread_count_ { thread_count }
    , suspend_ { suspend }
    , schedulers_ { thread_count, nullptr }
    , barrier_ { thread_count } {}

  const std::uint32_t thread_count_;
  const bool suspend_;
  std::atomic<std::uint32_t> counter_ = 0;
  std::vector<pooled_work_stealing*> schedulers_;
  boost::barrier barrier_;
};

class pooled_work_stealing : public boost::fibers::algo::algorithm {
private:
  std::shared_ptr<pool_ctx> pool_ctx_;

  std::uint32_t id_;
#ifdef BOOST_FIBERS_USE_SPMC_QUEUE
  boost::fibers::detail::context_spmc_queue rqueue_ {};
#else
  boost::fibers::detail::context_spinlock_queue rqueue_ {};
#endif
  std::mutex mtx_ {};
  std::condition_variable cnd_ {};
  bool flag_ { false };

public:

  static std::shared_ptr<pool_ctx>
  create_pool_ctx(std::uint32_t thread_count, bool suspend) {
    return std::make_shared<pool_ctx>(thread_count, suspend);
  }

  pooled_work_stealing(std::shared_ptr<pool_ctx> pc )
    : pool_ctx_ { pc }
    , id_ { pool_ctx_->counter_++ } {
      pool_ctx_->schedulers_[id_] = this;
      pool_ctx_->barrier_.wait();
    }

  ~pooled_work_stealing() {
    // Wait for all thread of the pool such that pointers in pool_ctx_
    // stay valid while still in use
    pool_ctx_->barrier_.wait();
  }

  pooled_work_stealing(pooled_work_stealing const&) = delete;
  pooled_work_stealing(pooled_work_stealing &&) = delete;

  pooled_work_stealing & operator=(pooled_work_stealing const&) = delete;
  pooled_work_stealing & operator=(pooled_work_stealing &&) = delete;

  void awakened(boost::fibers::context * ctx) noexcept override {
    if (!ctx->is_context(boost::fibers::type::pinned_context))
      ctx->detach();
    rqueue_.push(ctx);
  }

  context * pick_next() noexcept override {
    context * victim = rqueue_.pop();
    if (nullptr != victim) {
      boost::context::detail::prefetch_range(victim, sizeof(context));
      if (!victim->is_context(type::pinned_context)) {
        context::active()->attach(victim);
      }
    }
    else {
      std::size_t count = 0;
      std::size_t size = pool_ctx_->thread_count_;
      static thread_local std::minstd_rand generator { std::random_device{}() };
      std::uniform_int_distribution< std::uint32_t > distribution
        { 0, static_cast<std::uint32_t>(size - 1) };
      do {
        std::uint32_t id = 0;
        do {
          ++count;
          // Random selection of one logical CPU that belongs to the
          // local NUMA node
          id = distribution(generator);
          // Prevent stealing from own scheduler
        } while (id == id_);
        // Steal context from other scheduler
        victim = pool_ctx_->schedulers_[id]->steal();
      } while (nullptr == victim && count < size);
      if (nullptr != victim) {
        boost::context::detail::prefetch_range(victim, sizeof(context));
        BOOST_ASSERT(!victim->is_context(type::pinned_context));
        context::active()->attach(victim);
      }
    }
    return victim;
  }

  virtual boost::fibers::context * steal() noexcept {
    return rqueue_.steal();
  }

  bool has_ready_fibers() const noexcept override {
    return !rqueue_.empty();
  }

  void suspend_until(std::chrono::steady_clock::time_point const& time_point)
    noexcept override {
    if (pool_ctx_->suspend_) {
      if ((std::chrono::steady_clock::time_point::max)() == time_point) {
        std::unique_lock lk { mtx_ };
        cnd_.wait(lk, [&] { return flag_; });
        flag_ = false;
      }
      else {
        std::unique_lock lk { mtx_ };
        cnd_.wait_until(lk, time_point, [&] { return flag_; });
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

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_ALGO_POOLED_WORK_STEALING_H
