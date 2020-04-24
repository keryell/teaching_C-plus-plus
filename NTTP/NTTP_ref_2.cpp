#include "NTTP_ref.hpp"

// Can work with extern too in 1 file and not in the other one
inline S a;

void f() {
  // Would not work because of no static linkage
  // S a;

  U<a> u;
}
