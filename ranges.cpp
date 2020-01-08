#include <iostream>
#include <range/v3/all.hpp>

int main() {
  for (auto i : ranges::iota_view { 2, 10})
    std::cout << "For i=" << i << std::endl;
  for (auto [ x, y ] :
         ranges::views::cartesian_product(ranges::views::iota(0, 2),
                                          ranges::views::iota(1, 4)))
    std::cout << "x = " << x << " y = " << y << std::endl;

  /*
  // Infinite starting at 2
  for (auto i : ranges::views::iota(2))
    std::cout << "For i=" << i << std::endl;
  */

  static auto for_each_tile = [] (auto f) {
    for (auto [ x, y ] :
           ranges::views::cartesian_product(ranges::views::iota(0, 3),
                                            ranges::views::iota(0, 5)))
      f(x, y);
  };

  for_each_tile([] (int x, int y) {
                  std::cout << "x = " << x << " y = " << y << std::endl;
                });
}
