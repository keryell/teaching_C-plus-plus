// A 2D array without initialization
//
// https://godbolt.org/z/_pXo2s

#include <complex>
#include <cstdint>
#include <type_traits>

// Use a union to avoid initialization
template <typename T, std::size_t X, std::size_t Y>
union uninitialized_array {
  T data[X][Y];
  uninitialized_array() { /* No initializing... */ };
  // For lazy people, just pretend we can use this union as the internal value
  using data_type = decltype(data);
  operator data_type & () { return data; };
};

int main() {
  uninitialized_array<std::complex<std::int16_t>, 3, 5> a;

  // You can access the internal details, but it is tedious
  a.data[2][3] = { 2, 3 };

  // Because the customers deserve some laziness too
  a[1][2] = { 42, 314 };
  a[1][2] += a[2][3];
  return a[1][2].real();
}
