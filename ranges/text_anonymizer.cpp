/** \file

    Read some text grom stdin and generate random content to replace
    letters and digits

    \author Ronan Keryell
*/
#include <cctype>
#include <iostream>
#include <random>
#include <string>

#include <range/v3/all.hpp>

int main() {
  // To generates random characters in various categories
  std::ranlux24_base rng;
  std::uniform_int_distribution random_lower_case { 'a', 'z' };
  std::uniform_int_distribution random_upper_case { 'A', 'Z' };
  std::uniform_int_distribution random_digit('0', '9');

  for (auto line : ranges::getlines_view { std::cin }) {
    for (auto c : line)
      if (std::islower(c))
        std::cout << random_lower_case(rng);
      else if (std::isupper(c))
        std::cout << random_upper_case(rng);
      else if (std::isdigit(c))
        std::cout << random_digit(rng);
      else
        // Output any other characters unchanged
        std::cout << c;
    std::cout << std::endl;
    }
}
