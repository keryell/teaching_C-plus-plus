/* The Algorithm Design Manual

   Steven S. Skiena

   Second Edition

   p. 100

   3.10 Exercises

Applications of Tree Structures

3-10. [5] In the bin-packing problem, we are given n metal objects, each
weighing between zero and one kilogram. Our goal is to find the smallest
number of bins that will hold the n objects, with each bin holding one
kilogram at most.

• The best-fit heuristic for bin packing is as follows. Consider the
objects in the order in which they are given. For each object, place it
into the partially filled bin with the smallest amount of extra room after
the object is inserted.. If no such bin exists, start a new bin. Design an
algorithm that implements the best-fit heuristic (taking as input the n
weights w1 , w2 , ..., wn and outputting the number of bins used) in O(n
log n) time.

• Repeat the above using the worst-fit heuristic, where we put the next
object in the partially filled bin with the largest amount of extra room
after the object is inserted.
*/

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <set>


auto constexpr N = 100;
auto constexpr MAX_VAL = 1.0;
using value_type = float;


/** Store the bins in a priority queue giving the least element

    Each bin stores the used amount
*/
auto worst_fit_bin_packing = [] (auto &bins, auto elements) {
  std::cout << "Bin packing with worst fit heuristic" << std::endl << std::endl;

  for (auto const e : elements) {
    std::cout << "Insert value " << e << std::endl;
    if (bins.empty() || (bins.top() + e > MAX_VAL)) {
      std::cout << "There is no bin or the most empty has not enough room,"
                << " so create a new bin for this element" << std::endl;
      bins.push(e);
    }
    else {
      // Pop the top bin
      auto top = bins.top();
      bins.pop();
      // And put it again in the priority queue with the element e
      bins.push(top + e);
      std::cout << "Add it to bin " << top << " to have " << top + e
                << std::endl;
    }
  }
};


/* Store the bins in a multiset because we need to find the best fit bin
   and there may be 2 bins with the same value.

   Each bin actually stores the available amount of space, not the used
   amount.

   By using the default set ordering, the element are sorted by increasing
   amount of available space */
auto best_fit_bin_packing = [] (auto &bins, auto elements) {
  std::cout << "Bin packing with best fit heuristic" << std::endl << std::endl;

  for (auto const e : elements) {
    std::cout << "Insert value " << e << std::endl;
    // for(auto b : bins)
    //   std::cout << " " << b;
    // std::cout << std::endl;

    // Get an interator on a bin
    typename std::remove_reference<decltype(bins)>::type::iterator b;

    /* Look up the element with the requested value if it does exist
           or the element with just the value above, if any */
    if ((b = bins.lower_bound(e)) == bins.end()) {
      std::cout << "There is no bin with enough room, "
                << "so create a new bin for this element with "
                << (MAX_VAL - e) << " free" << std::endl;
      bins.insert(MAX_VAL - e);
    }
    else {
      /* There is already a bin for storing the value. Remove its place */
      auto new_value = *b  - e;
      std::cout << "Add it to free bin " << *b << " to have " << new_value
                << " free" << std::endl;
      // Since the set values are immutable, remove the old bin...
      bins.erase(b);
      // ... and insert the new value back
      bins.insert(new_value);
    }
  }
};


int main() {
  // To generate the various random generators in the following
  std::default_random_engine rng;
  // To generate random numbers between 0.0 and 1.0
  std::uniform_real_distribution<value_type> rand_mo;

  // Create a vector of N metal objects randomly initialized
  std::vector<value_type> metal_objects(N);
  std::generate(std::begin(metal_objects),
                std::end(metal_objects),
                [&] { return rand_mo(rng); });

  // Bin packing with worst fit heuristic

  std::priority_queue<value_type,
                      std::vector<value_type>,
                      std::greater<value_type>> worst_fit_bins;
  worst_fit_bin_packing(worst_fit_bins, metal_objects);
  std::cout << worst_fit_bins.size() << " bins used with content:" << std::endl;
  while (!worst_fit_bins.empty()) {
    auto b = worst_fit_bins.top();
    worst_fit_bins.pop();
    std::cout << b << std::endl;
  }

  // Bin packing with best fit heuristic
  std::multiset<value_type> best_fit_bins;
  best_fit_bin_packing(best_fit_bins, metal_objects);
  std::cout << best_fit_bins.size() << " bins used with content:" << std::endl;
  for (auto const b : best_fit_bins)
    std::cout << b << std::endl;

  return 0;
}
