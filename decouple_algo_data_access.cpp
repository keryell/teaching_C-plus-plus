/** \file

    Experiment with decoupling algorithm from data access
*/
#include <iostream>
#include <vector>
#include <tuple>
#include <utility>
#include <range/v3/all.hpp>

constexpr auto n = 10;

template <typename DataAccess>
struct program {
  DataAccess &d;

  program(DataAccess &da) : d { da } {}

  auto operator()() {
    int result = 0;
    for (int i = 0; i < n; ++i)
      result += d.x(i)*d.y(i);
    return result;
  }
};

struct struct_of_array {
// Work only with the development version, so replace by 10 lines of code...
//  static auto inline x = ranges::view::iota(0, n) | ranges::to<std::vector>;
//  static auto inline y = ranges::view::iota(1, n) | ranges::to<std::vector>;
  std::vector<int> x;
  std::vector<int> y;

  struct_of_array() {
  std::vector<int> x(n);
  std::iota(x.begin(), x.end(), 0);
  this->x = x;
  std::vector<int> y(n);
  std::iota(y.begin(), y.end(), 1);
  this->y = y;
  }
};

template <typename Data>
struct data_access {
  Data &d;

  data_access(Data &d) : d { d } {}

  auto &x(size_t i) { return d.x[i]; }

  auto &y(size_t i) { return d.y[i]; }
};

struct array_of_struct {
    static auto inline v = std::vector<std::tuple<int,int>>(10);
};

template <>
struct data_access<array_of_struct> {
  array_of_struct &d;

  data_access(array_of_struct &d) : d { d } {}

  auto &x(size_t i) { return std::get<0>(d.v[i]); }

  auto &y(size_t i) { return std::get<1>(d.v[i]); }
};

template <template <typename DataAccess> typename Algo, typename Data>
auto weave(Data &d) {
  data_access<Data> da { d };
  return Algo<data_access<Data>>(da);
}

template <typename Src, typename Dst>
void convert_my_data(Src &s, Dst &d) {
  data_access<Src> sa { s };
  data_access<Dst> da { d };

  for (int i = 0; i < n; ++i) {
    da.x(i) = sa.x(i);
    da.y(i) = sa.y(i);
  }
}

int main() {
    struct_of_array soa;
    array_of_struct aos;
    convert_my_data(soa, aos);
    std::cout << "Structure of array: " << weave<program>(soa)() << std::endl;
    std::cout << "Array of structure: " << weave<program>(aos)() << std::endl;
}
