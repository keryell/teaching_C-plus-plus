// A wrapper type to skip initialization
//
// https://godbolt.org/z/V3wCcS

#include <complex>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// Use a union to avoid initialization
template <typename T>
union remove_initialization {
  using value_type = T;
  T data;
  remove_initialization() { /* No initializing here... */ };
  // For lazy people, just pretend we can use this union as the internal value
  // Not perfect, but sill waiting for https://isocpp.org/blog/2016/02/a-bit-of-background-for-the-operator-dot-proposal-bjarne-stroustrup
  operator value_type & () { return data; };
};

struct boom_on_construct {
  // This assignment is actually made by the constructor
  int e = 3;
  boom_on_construct() {
    std::abort();
  }
};

int main() {
  remove_initialization<std::complex<std::int16_t>[3][5]> a;

  //boom_on_construct boom;
  remove_initialization<boom_on_construct> no_boom;
  // Read uninitialized memory because the constructor is skipped
  std::cout << "Probably not 3: " << no_boom.data.e << std::endl;
  // To avoid the .data, we can get the real boom_on_construct underneath
  boom_on_construct & simpler_no_boom = no_boom;
  // Read uninitialized memory because the constructor is skipped
  std::cout << "Probably not 3: " << simpler_no_boom.e << std::endl;

  // You can use .data with arrays too, but it is tedious
  a.data[2][3] = { 2, 3 };

  // Because the customers deserve also some laziness
  a[1][2] = { 42, 314 };
  a[1][2] += a[2][3];
  std::cout << "This should be 44: " << a[1][2].real() << std::endl;
}
