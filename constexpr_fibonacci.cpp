/* A constexpr implementation of Fibonacci computed at compile time with
   C++14
 */

#include <iostream>
constexpr auto fibonacci(int v) {
  long long int u_n_minus_1 = 0;
  auto u_n = u_n_minus_1 + 1;
  for (int i = 1; i < v; ++i) {
    auto tmp = u_n;
    u_n += u_n_minus_1;
    u_n_minus_1 = tmp;
  }
  return u_n;
}


int main() {
  constexpr auto result = fibonacci(80);
  std::cout << result << std::endl;
  return 0;
}
