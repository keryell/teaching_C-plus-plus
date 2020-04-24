/* The Algorithm Design Manual

   Steven S. Skiena

   Second Edition

   p. 271

   7.11 Exercises

Interview Problems

7-14. [4] Write a function to find all permutations of the letters in a
particular string.
*/

#include <cstddef>
#include <iostream>
#include <utility>


/** Permute recursively all the letters in a string

    Note it does not deal with repeated letters and it this case if a
    letter is repeated r times, the strings will be repeated r! times.

    The issue with this version is that input.size() may be evaluated a lot
    if the compiler cannot figure out that the string size is constant.

    \todo Reverse the iteration direction to avoid the input.size() issue
    without passing the string length in parameter.
*/
void permute(std::string &input, std::size_t level = 0) {
  if (level < input.size())
    for (auto i = level; i < input.size(); ++i) {
      std::swap(input[level], input[i]);
      permute(input, level + 1);
    }
  else
    std::cout << input << std::endl;
}


int main() {
  std::string input { "simple" };
  permute(input);

  input = "This is quite boring!";
  permute(input);

  return 0;
}
