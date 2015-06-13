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


auto constexpr N = 100;
auto constexpr MAX_VAL = 1.0;
using value_type = float;

void worst_fit_bin_packing(auto &bins, auto elements) {
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
}


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

  // Store the bins in a priority queue giving the least element
  std::priority_queue<value_type,
                      std::vector<value_type>,
                      std::greater<value_type>> worst_fit_bins;
  worst_fit_bin_packing(worst_fit_bins, metal_objects);
  std::cout << worst_fit_bins.size() << " bins used with content:" << std::endl;
  while (!worst_fit_bins.empty()) {
    auto e = worst_fit_bins.top();
    worst_fit_bins.pop();
    std::cout << e << std::endl;
  }

  return 0;
}
