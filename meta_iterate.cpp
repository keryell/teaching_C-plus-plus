#include <cstddef>
#include <iostream>
#include <iterator>
#include <utility>
#include <boost/hana.hpp>

template<int ID>
int func(short a, short b) {
  static int reg = 0;
  reg = reg + a*b;

  return reg;
}

/// Iterate at compile time on all the indices J in { 0, 1, 2, 3, 4 }
template <std::size_t... J>
void iterate(int res[5], int i, short a[5][10], short b[5][10],
             std::index_sequence<J...>) {
  // Expand the call for each J. Use assignment just to have some
  // expansion syntax
  auto junk = { (res[J] = func<J>(a[J][i], b[J][i]))... };
}

void my_function() {
  short a[5][10], b[5][10];
  int res[5];

  for (int i = 0; i < 10; ++i)
    // std::make_index_sequence<5>{} creates a meta-sequence 0, 1, 2, 3, 4
    iterate(res, i, a, b, std::make_index_sequence<5>{});
}

template <int first, int last>
void example(int i) {
  std::cout << "For i=" << i << " first is "
            << first << " and last is " << last << std::endl;
}

int main() {
  my_function();

  // https://godbolt.org/z/YCkgSd
  static constexpr int decimsize[] = { 16, 8, 4, 2, 1 };
  // In C++20 it would be shorter :-)
  // auto constexpr NUM_FIR = std::size(decimsize)
  // Do it the boring way in the meantime:
  int constexpr NUM_FIR = sizeof(decimsize)/sizeof(decimsize[0]);

  boost::hana::int_c<NUM_FIR>.times.with_index([&] (auto i) {
    if constexpr (i > 0 && i < NUM_FIR - 1)
      example<decimsize[i - 1], decimsize[i + 1]>(i);
  });
}
