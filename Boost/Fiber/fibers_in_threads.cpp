/** \file

    Small example showing how to use some threads to simulate a
    network with some routing elements running in fibers running in
    different threads.
*/

#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include <boost/thread/barrier.hpp>
#include <boost/fiber/all.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>

using namespace std::literals;

auto constexpr capacity = 4;
auto constexpr message_length = 10;
auto constexpr num_threads = 3;
/// Whether the fibers suspend their thread or not
auto constexpr suspend = false;
//auto constexpr suspend = true;

using packet = int;

boost::barrier starting_block { num_threads };
boost::barrier finish_line { num_threads };

//auto console = spdlog::stdout_logger_mt("console");
std::shared_ptr<spdlog::logger> console;

auto logging = [] (auto message, auto... args) {
  auto format = "Fiber {} on thread {}: "s + message;
  console->info(format.c_str(), boost::this_fiber::get_id(),
                std::this_thread::get_id(), args...);
};


struct router {
  /// Ingress packet queue
  boost::fibers::buffered_channel<packet> ingress { capacity };

  /// Egress packet queue
  boost::fibers::buffered_channel<packet> egress { capacity };

  /// Fiber used as a data mover between queues
  boost::fibers::fiber f;

  /// Handle the thread running the routing fibers
  std::future<void> fiber_runner;

  router() {
    fiber_runner = std::async(std::launch::async, [&] {
      boost::fibers::use_scheduling_algorithm
        <boost::fibers::algo::work_stealing>(num_threads, suspend);
      starting_block.wait();
      std::cout << "Starting thread " << std::this_thread::get_id()
                << " running router fibers" << std::endl;
      f = { /* Use explicit dispatch to have the scheduler starting right
               away instead of waiting for the join() (called only in the
               destructor...) to launch it */
           boost::fibers::launch::dispatch,
           [&] {
             logging("router {} starting with buffered_channel {}",
                     (void *)this, (void *)&ingress);
             for (int i = 0; i < message_length; ++i) {
                logging("router {} reading from buffered_channel {}",
                        (void *)this, (void *)&ingress);
               auto v = ingress.value_pop();
               logging("router {} routing data value {}", (void *)this, v);
               egress.push(v);
             }
             logging("router {} is shutting down", (void *)this);;
           }
      };
      finish_line.wait();
    });
  }

//    std::vector<std::shared_ptr<router_port>> outputs;
  void write(int v) {
    logging("writing {} to router {} on buffered_channel {}",
            v, (void *)this, (void *)&ingress);
    ingress.push(v);
  }


  int read() {
    logging("reading from router {} from buffered_channel {}...",
            (void *)this, (void *)&ingress);
    return egress.value_pop();
  }


  /// Destructor handling the correct infrastructure shutdown
  ~router() {
    // Wait for shutdown to avoid calling std::terminate on destruction...
    f.join();
    fiber_runner.get();
  }
};

int main() {
  router r;
  spdlog::init_thread_pool(10000, 1);
  console = spdlog::create_async<spdlog::sinks::stdout_sink_mt>("console");

  // Launch some producers
  auto producer = std::async(std::launch::async, [&] {
      boost::fibers::use_scheduling_algorithm
        <boost::fibers::algo::work_stealing>(num_threads, suspend);
      starting_block.wait();
      logging("starting producer");

      auto prod = [&] (int id) {
        return [&, id] {
          for (int i = id*1000; i < message_length + id*1000; ++i)
           r.write(i);
        };
      };
      // Delay the launch time to maximize the raciness
      boost::fibers::fiber p1 { boost::fibers::launch::post, prod(0) };
      boost::fibers::fiber p2 { boost::fibers::launch::post, prod(1) };
      p1.join();
      p2.join();
      finish_line.wait();
    });

  // Launch some consumers
  auto consumer = std::async(std::launch::async, [&] {
      boost::fibers::use_scheduling_algorithm
        <boost::fibers::algo::work_stealing>(num_threads, suspend);
      starting_block.wait();
      logging("starting consumer");
      auto cons = [&] (int id) {
        return [&, id] {
          for (int i = id; i < message_length; ++i) {
            auto v = r.read();
            logging("consumer {} read {} from router {}", id, v, (void *)&r);
          }
        };
      };
      // Delay the launch time to maximize the raciness
      boost::fibers::fiber c1 { boost::fibers::launch::post, cons(0) };
      boost::fibers::fiber c2 { boost::fibers::launch::post, cons(1) };
      c1.join();
      c2.join();
      finish_line.wait();
    });

  // Wait for everybody to finish
  producer.get();
  consumer.get();
}
