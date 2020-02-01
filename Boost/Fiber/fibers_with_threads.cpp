/** \file

    Small example showing how to use some threads to simulate a
    network with some routing elements running in fibers in a specific
    thread (not in different threads).
*/

#include <cassert>
#include <future>
#include <iostream>
#include <thread>
#include <boost/fiber/all.hpp>

auto constexpr capacity = 4;
auto constexpr message_length = 10;

using packet = int;

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
      std::cout << "Starting thread " << std::this_thread::get_id()
                << " running router fibers" << std::endl;
      f = { /* Use explicit dispatch to have the scheduler starting right
               away instead of waiting for the join() (called only in the
               destructor...) to launch it */
           boost::fibers::launch::dispatch,
           [&] {
             std::cout << "Thread " << std::this_thread::get_id()
                       << " router " << this << " on fiber "
                       << boost::this_fiber::get_id()
                       << " starting with buffered_channel "
                       << &ingress << std::endl;
             for (int i = 0; i < message_length; ++i) {
               std::cout << "router " << this << " on fiber "
                         << boost::this_fiber::get_id()
                         << " reading from buffered_channel "
                         << &ingress << " ..." << std::endl;
               auto v = ingress.value_pop();
               std::cout << "router " << this << " on fiber "
                         << boost::this_fiber::get_id()
                         << " routing data value " << v << std::endl;
               egress.push(v);
             }
             std::cout << "router " << this << " on fiber "
                       << boost::this_fiber::get_id()
                       << " shutting down" << std::endl;
           }
      };
    });
  }

//    std::vector<std::shared_ptr<router_port>> outputs;
  void write(int v) {
        std::cout << "Thread " << std::this_thread::get_id()
                  << " fiber " << boost::this_fiber::get_id()
                  << " is writing " << v << " to router " << this
                  << " on buffered_channel " << &ingress << std::endl;
    ingress.push(v);
  }


  int read() {
    std::cout << "Thread " << std::this_thread::get_id()
              << " fiber " << boost::this_fiber::get_id()
              << " is reading from router " << this
              << " from buffered_channel " << &egress << std::endl;
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

  // Launch a producer
  auto producer = std::async(std::launch::async, [&] {
      std::cout << "Starting producer as thread "
                << std::this_thread::get_id()
                << " on fiber " << boost::this_fiber::get_id() << std::endl;
      for (int i = 0; i < message_length; ++i)
        r.write(i);
    });

  // Launch a consumer
  auto consumer = std::async(std::launch::async, [&] {
      std::cout << "Starting consumer as thread "
                << std::this_thread::get_id()
                << " on fiber " << boost::this_fiber::get_id() << std::endl;
      for (int i = 0; i < message_length; ++i) {
        auto v = r.read();
        std::cout << "Consumer read " << v << std::endl;
        // Check we read the correct value
        if (v != i)
          std::cerr << "Consumer read " << v
                    << " instead of " << i << std::endl;
      }
    });

  // Wait for everybody to finish
  producer.get();
  consumer.get();

}
