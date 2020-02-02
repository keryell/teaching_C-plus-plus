/* Experiment with annotations

   clang++-11 -std=c++2a -O0 -S -emit-llvm -mllvm -disable-llvm-optzns \
   annotate.cpp

  %5 = bitcast %struct.s* %2 to i8*
  call void @llvm.var.annotation(i8* %5, i8* getelementptr inbounds
  ([14 x i8], [14 x i8]* @.str, i32 0, i32 0), i8* getelementptr
  inbounds ([23 x i8], [23 x i8]* @.str.1, i32 0, i32 0), i32 11)

  Attach to an alloca the annotation string, the file name @.str.1 and
  the line number 11
*/

// "structure_definition" is lost
struct __attribute__((annotate("structure_definition"))) s {
  /* This goes into the constructor @_ZN1sC2Ev */
  // @llvm.ptr.annotation.p0i8 "int_before_var" on v
  // @llvm.ptr.annotation.p0i8 "int_after_var" on the previous annotation!
  // 0 is written in the memory pointed by the return of the last annotation!
  // .p0i8 means pointer to i8 in address space 0
  mutable int __attribute__((annotate("int_before_var")))
  v __attribute__((annotate("int_after_var"))) = 0;

  // 2 @llvm.global.annotations on @_ZN1s3siaE
  static inline auto __attribute__((annotate("sia_before")))
  sia __attribute__((annotate("sia_after"))) = 42;

  // Annotations on constexpr members are lost even when the value is used
  static constexpr auto __attribute__((annotate("sca_before")))
  sca __attribute__((annotate("sca_after"))) = 123;

  auto &get_ref() { return  v; }
};


template <typename T>
struct decorate {
  T v;
  decorate() {
    int __attribute__((annotate("in_constructor"))) nothing;
  }

  operator auto&() {
    return v;
  }

/*
  operator T&() {
    return v;
  }
*/
};


template <typename name, typename Func>
void kernel_single_task(Func kernelFunc
                        __attribute__((annotate("kernelFunc")))) {
  // Keep for fun
  name * __attribute__((annotate("name"))) p;
  kernelFunc();
}

template <int bits>
struct apint {
  int v __attribute__((annotate("v")));
  static auto inline b __attribute__((annotate("bits"))) = bits;
  apint (int v) : v { v } {}

  apint() = default;

  operator int&() { return v; }
};

template <int value, typename Base>
struct add_annotation {
  static inline auto a __attribute__((annotate("add_annotation"))) = value;
  static inline __attribute__((annotate("base")))
  Base __attribute__((annotate("base")))
  *__attribute__((annotate("base"))) base;
  add_annotation() {
    auto __attribute__((annotate("x"))) x = 3;
    base = nullptr;
  }
};


struct value : add_annotation<37,value> {
  float v;
};

template <int value>
struct generic_value : add_annotation<value,generic_value<value>> {
  float v;
  static inline double  __attribute__((annotate("double"))) *d = nullptr;

};

int main() {
  s a;
  // @llvm.var.annotation on b
  s __attribute__((annotate("structure_var"))) b;
  a.get_ref() = 1;
  b.get_ref() = 1;

  // All f1 f2 f3 annotate f with @llvm.var.annotation
  float __attribute__((annotate("f1")))
    __attribute__((annotate("f2"))) __attribute__((annotate("f3"))) f;
  double __attribute__((annotate("bd1")))
    __attribute__((annotate("bd2"))) __attribute__((annotate("bd3")))
    d __attribute__((annotate("d1")))
    __attribute__((annotate("d2"))) __attribute__((annotate("d3")));

//  auto u = a.sia;
  auto v = a.sca;

  decorate<int> c;
  int cc = c;
  cc  = 3;

  kernel_single_task<class kernel_function>([]() {
  });

  value val;
  auto val2 = val;

  generic_value<42> gv;

  apint<3> b3 = 2;
  auto b4 = b3;
  int i = b4;

  return i;
}
