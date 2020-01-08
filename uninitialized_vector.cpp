/* Experiment with allocator to remove default initialization in
   containers and also use page-aligned memory

   https://godbolt.org/z/W_bdM-
*/

/* Inspiration from
   https://stackoverflow.com/questions/15097783/value-initialized-objects-in-c11-and-stdvector-constructor/15119665#15119665

template<typename T>
struct DefaultInitAllocator {
    template<typename U>
    void construct(U* p)
    { ::new (static_cast<void*>(p)) U; }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    { ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...); }

    // ... rest of the allocator interface
};

// ...
typedef std::vector<int, DefaultInitAllocator<int>> DefaultInitVectorInt;
*/

#include <cstdlib>
#include <iostream>
#include <vector>

// https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028912#21028912
// Allocator adaptor that interposes construct() calls to
// remove value initialization and use page alignment
template <typename T, typename A = std::allocator<T>>
class ua_allocator : public A {
  using a_t = std::allocator_traits<A>;

public:

  template <typename U> struct rebind {
    using other = ua_allocator<U,typename a_t::template rebind_alloc<U>>;
  };

  // Inherit from the allocator constructors
  using A::A;

  // Replace the default construction.
  // It is noexcept since there is nothing to do here
  template <typename U>
  void construct(U* ptr) noexcept {
    //noexcept(std::is_nothrow_default_constructible<U>::value) {
    /* Skip the default construction
       ::new(static_cast<void*>(ptr)) U;
    */
    std::cout << "Default construction" << std::endl;
  }

  /* Note that if you do not need to display the error message you can
     remove completely this definition from here since it will be
     synthesized for you by the allocator trait
     std::allocator_traits<Alloc>::construct
     https://en.cppreference.com/w/cpp/memory/allocator_traits/construct
  */
  template <typename U, typename...Args>
  void construct(U* ptr, Args&&... args) {
    a_t::construct(static_cast<A&>(*this),
                   ptr, std::forward<Args>(args)...);
    std::cout << "Construction with parameters" << std::endl;
  }

  // Use an 4 KiB aligned allocator
  T* allocate(std::size_t num) {
    void* ptr;
    std::cout << "Allocation" << std::endl;
    if (posix_memalign(&ptr, 4096, num*sizeof(T)))
      throw std::bad_alloc();
    return static_cast<T*>(ptr);
  }

  // The corresponding deallocator
  void deallocate(T* p, std::size_t num) {
    std::cout << "Deallocation" << std::endl;
    free(p);
  }
};

// Some type to show more than with an int whether it is initialized or not
struct element {
  int e = 42;

  element(int v) : e { v } {}

  element() = default;

  operator int &() { return e; }
};

int main() {
  std::vector<int> v(10);
  std::cout << "v[5] = " << v[5] << std::endl;
  std::vector<int, ua_allocator<int>> v_dia(2);
  std::cout << "v_dia[1] = " << v_dia[1] << std::endl;
  std::vector<int, ua_allocator<int>> v_dia_6(3, 6);
  std::cout << "v_dia_6[1] = " << v_dia_6[1] << std::endl;

  std::vector<element> ve(10);
  std::cout << "ve[5] = " << ve[5] << std::endl;
  std::vector<element, ua_allocator<element>> ve_dia(2);
  std::cout << "ve_dia[1] = " << ve_dia[1] << std::endl;
  std::vector<element, ua_allocator<element>> ve_dia_6(3, 6);
  std::cout << "ve_dia_6[1] = " << ve_dia_6[1] << std::endl;
}
