#include <cctype>
#include <iostream>
#include <string>
#include <range/v3/all.hpp>

/// Capitalize a string by changing its first character to uppercase
std::string capitalize(const std::string_view& input) {
  auto r = ranges::views::all(input);
  return ranges::views::concat
    (r | ranges::views::take(1)
     | ranges::views::transform([] (auto c) { return std::toupper(c); }),
     r | ranges::views::drop(1))
    | ranges::to<std::string>;
}

int main() {
  std::cout << capitalize("salut") << std::endl;
  std::cout << capitalize("") << std::endl;
  std::cout << capitalize("i") << std::endl;
  std::cout << capitalize("ok") << std::endl;
  std::cout << capitalize(std::string { "CouCou" }) << std::endl;
}
