#include <array>
#include <iostream>

auto constexpr FFT_D = 4;
using Matrix = std::array<int, FFT_D>;

int main() {
  Matrix a;
  a[2] = 3;
  // Make a new copy on the stack
  auto b = a;

  std::cout << b[2] << std::endl;

  // Atrocious explicit memory allocation
  auto p = new Matrix;

  // Copy all the elements of a to *p
  *p = a;
  std::cout << (*p)[2] << std::endl;

  // We are getting bored with this Christmas' tree programming style
  // with all these stars everywhere, make an alias
  auto &nicer = *p;

  std::cout << nicer[2] << std::endl;

  b[2] = 1;

  std::cout << nicer[2] << std::endl;

  // Quite nicer notation!
  nicer = b;

  std::cout << nicer[2] << std::endl;

  // Do not forget to free the memory at some point because we went
  // the Dark Side of the Force
  delete &nicer;
  // Could be instead:
  // delete p;
}
