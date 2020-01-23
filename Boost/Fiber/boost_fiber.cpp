#include <cassert>
#include <iostream>
#include <boost/fiber/all.hpp>

auto constexpr capacity = 4;
auto constexpr message_length = 10;

int main() {
  boost::fibers::buffered_channel<int> c { capacity };

  // Test boost::fibers::buffered_channel behaviour
  for (int i = 10; i < 10 + message_length; ++i) {
    c.push(i);
    auto receive = c.value_pop();
    assert(i == receive);
  }

  // Test boost::fibers::fiber
  boost::fibers::fiber receiver {
    /* Use explicit dispatch to have the scheduler starting right away
       instead of waiting for the join() to launch it */
    boost::fibers::launch::dispatch,
    [&] {
      std::cerr << "Fiber receiver " << boost::this_fiber::get_id()
                << " started" << std::endl;
      for (int i = 0; i < message_length; ++i) {
        auto v = c.value_pop();
        // Just adding the message dead-locks
        std::cerr << "Fiber receiver " << boost::this_fiber::get_id()
                  << " read " << v << std::endl;
        assert(i == v);
      }
    }};

  boost::fibers::fiber sender { //boost::fibers::launch::dispatch,
    [&] {
      std::cerr << "Fiber sender " << boost::this_fiber::get_id()
                << " started" << std::endl;
       for (int i = 0; i < message_length; ++i) {
        std::cerr << "Fiber sender " << boost::this_fiber::get_id()
                  << " pushing " << i << std::endl;
        c.push(i);
      }
    }};

  std::cerr << "Go to sleep to see when the fibers really start" << std::endl;
  sleep(1);

  sender.join();
  receiver.join();
}
