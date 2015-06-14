/* Experiment with a set to understand limit conditions */

#include <iostream>
#include <set>

using value_type = float;
//using set_type = std::multiset<value_type>;
using set_type = std::set<value_type>;

void try_set(value_type v, set_type s) {
  std::cout << "Look for value " << v << std::endl;
  auto it_low = s.lower_bound(v);
  auto it_up = s.upper_bound(v);
  std::cout << "it_low is end: " << (it_low == s.end()) << std::endl;
  std::cout << "it_up is end: " << (it_up == s.end()) << std::endl;
  /* Interestingly it does not crash when it points to end() with the
     implementation I tried */
  std::cout << "it_low point to " << *it_low << std::endl;
  std::cout << "it_up point to " << *it_up << std::endl << std::endl;
}


int main() {
  set_type my_set { 1, 2, 3, 4, 5 };
  try_set(-10, my_set);
  try_set(1, my_set);
  try_set(2, my_set);
  try_set(2.5, my_set);
  try_set(5, my_set);
  try_set(50, my_set);
}
