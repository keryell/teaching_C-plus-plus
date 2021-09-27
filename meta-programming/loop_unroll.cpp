/** \file

    Implement a compile-time loop to unroll code

    Rely on C++17, so compile at least with -std=c++17 or -std=c++1z
*/
#include <array>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>


/** Instantiate a loop body for all the given indices

    \param[in] loop_body is the function to iterate. It takes the
    compile-time loop-index
*/
template <typename Body, std::size_t... I>
void instantiate(Body loop_body, std::index_sequence<I...>) {
  // Expand the call for each I with a C++17 folding on ","
  auto junk = (loop_body(std::integral_constant<std::size_t, I> {}), ..., 0);
  // Silent the "unused warning"
  static_cast<void>(junk);
}

/** Unroll a loop

    \tparam N is the number of iteration

    \param[in] loop_body is the function to iterate. It takes the
    compile-time loop-index
*/
template <std::size_t N, typename Body>
void loop_unroll(Body loop_body) {
  // Generate all the indices and instantiate the loop-body with them
  instantiate(loop_body, std::make_index_sequence<N> {});
}

int main() {
  loop_unroll<42>([&] (auto i) {
    // Use i as usual
    std::array<int, i> a;
    std::cout << "i = " << i << ", sizeof(a) = " << sizeof(a) << std::endl;
  });
}
