/* The Algorithm Design Manual

   Steven S. Skiena

   Second Edition

   p. 271

   7.11 Exercises

Interview Problems

7-14. [4] Write a function to find all permutations of the letters in a
particular string.
*/

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <utility>


/** Permute recursively all the letters in a string

    Use a back-tracking approach by recursively constructing a solution by
    picking the character in the set of available characters
*/
template <typename String, typename CharMap>
void perm(String &permutation,
          CharMap &char_map,
          std::size_t remaining_char) {
  /* If the permutation string has the right size, print it and stop the
     recursion */
  if (remaining_char == 0)
    std::cout << permutation << std::endl;
  else
    // For each character to try
    for (auto &i : char_map) {
      // If there is still an instance of this character to use
      if (i.second != 0) {
        // Since we use this letter, remove 1 instance
        i.second--;
        // Append the character to the permutation string
        permutation += i.first;
        perm(permutation, char_map, remaining_char - 1);
        // Trim the last character
        permutation.resize(permutation.size() - 1);
        // Restore the letter instance
        i.second++;
      }
    }
}


/** Initialize the permutation recursion
 */
template <typename String>
void permute(String &input) {
  std::map<typename String::value_type, std::size_t> char_map;

  // Construct an histogram with the number of each character
  for (auto c : input)
    char_map[c]++;

  String permutation;
  perm(permutation, char_map, input.size());
}


int main() {
  std::string input { "repeat" };

  // Compute the permutations using the STL

  // First display the increasing permutations from the starting point
  auto perm = input;
  do {
    std::cout << perm << std::endl;
  } while (std::next_permutation(perm.begin(), perm.end()));

  // The display the decreasing permutations from the starting point
  perm = input;
  while (std::prev_permutation(perm.begin(), perm.end()))
    std::cout << perm << std::endl;

  // Then use a home-brewed version
  permute(input);

  return 0;
}


/*
    # Some Emacs stuff:
    # Add some on-the-fly spell checking plus the normal programming mode
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
