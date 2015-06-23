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
          CharMap &char_map) {
  /* If there are no longer any character to use, print the permutation
     and stop the recursion */
  if (char_map.empty())
    std::cout << permutation << std::endl;
  else
    /* For each character to try

       Do not use a range-based for loop because we modify the map on the
       fly and need to manage the iterator */
    for (auto i = char_map.begin(); i != char_map.end(); ++i) {
      // Append the character (the key) to the permutation string
      permutation += i->first;
      // Since we use this letter, remove 1 instance (which is the map value)
      if (--(i->second) == 0) {
        /* If there is no longer this character, remove it from the map to
           avoid a test for it in subsequent recursions */
        char_map.erase(i);
        // Recurse with one character less
        perm(permutation, char_map);
        /* Restore the letter instance and restore the iterator
           to point again to this element */
        i = char_map.insert(std::make_pair(permutation[permutation.size() - 1],
                                           1)).first;
      }
      else {
        // Recurse with one character less
         perm(permutation, char_map);
        // Restore the letter instance
        ++(i->second);
      }
      // Trim the last character we added before
      permutation.resize(permutation.size() - 1);
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
  perm(permutation, char_map);
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
