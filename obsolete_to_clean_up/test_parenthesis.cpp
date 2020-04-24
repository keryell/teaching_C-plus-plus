/* The Algorithm Design Manual

   Steven S. Skiena

   Second Edition

   p. 98

   3.10 Exercises

Stacks, Queues, and Lists

3-1. [3] A common problem for compilers and text editors is determining
whether the parentheses in a string are balanced and properly nested. For
example, the string ((())())() contains properly nested pairs of
parentheses, which the strings )()( and ()) do not. Give an algorithm that
returns true if a string contains properly nested and balanced
parentheses, and false if otherwise. For full credit, identify the
position of the first offending parenthesis if the string is not properly
nested and balanced.
*/

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>


void check(const std::string &s) {
  int nested_level = 0;
  for (size_t i = 0; i != s.size(); ++i) {
    auto c = s[i];
    switch (c) {
    case '(':
      ++nested_level;
      break;
    case ')':
      if (--nested_level < 0) {
        std::cerr << "Unbalanced ')' at position " << i << std::endl;
        return;
      }
      break;
    default:
      throw std::runtime_error("Not a '(' or ')'");
    }
  }
}


int main() {

  std::string input;

  // To throw an exception on reading if something gets wrong, such as EOF
  std::cin.exceptions(std::iostream::failbit
                      | std::iostream::badbit
                      | std::iostream::eofbit);

  for (;;) {
    std::cout << "Enter a parenthesis expression:" << std::endl;
    std::cin >> input;
    check(input);
  }

  return 0;
}
