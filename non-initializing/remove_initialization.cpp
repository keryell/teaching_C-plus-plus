// A wrapping type to skip initialization
//
// https://godbolt.org/z/eA2D6F

#include <complex>
#include <cstdint>
#include <type_traits>

// Use a union to avoid initialization
template <typename T>
union remove_initialization {
  using value_type = T;
  T data;
  remove_initialization() { /* No initializing here... */ };
  // For lazy people, just pretend we can use this union as the internal value
  operator value_type & () { return data; };
};

int main() {
  remove_initialization<std::complex<std::int16_t>[3][5]> a;

  // You can access the internal details, but it is tedious
  a.data[2][3] = { 2, 3 };

  // Because the customers deserve some laziness too
  a[1][2] = { 42, 314 };
  a[1][2] += a[2][3];
  return a[1][2].real();
}
