// https://godbolt.org/z/g8-8TA
// make CXX=clang++-10 CXXFLAGS="-O3 -std=c++2a" quizz
#include <algorithm>
#include <iostream>
#include <numeric>

// Convert digits into base-10 number
auto base_10_value = [] (auto... digits) {
  std::array a { digits... };
  return std::accumulate(a.cbegin(), a.cend(), 0,
                         [] (auto sum, auto x) { return sum*10 + x; });
};

int main() {
  std::array<int, 10> digits;
  std::iota(digits.begin(), digits.end(), 0);
  auto & [ B, C, E, H, O, S, T, U, _8, _9 ] = digits;
  decltype(digits)::value_type max = 0;
  do {
    auto a = base_10_value(S, T, U, B);
    auto b = base_10_value(T, O, E);
    auto r = base_10_value(O, U, C, H);
    if (a + b == r && r > max)
      max = r;
  } while (std::next_permutation(digits.begin(), digits.end()));
  std::cout << max << std::endl;
}
