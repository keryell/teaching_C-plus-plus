#include <iostream>

struct S {};

template <auto & s>
struct U {
  U() { std::cout << &s << std::endl; }
};
