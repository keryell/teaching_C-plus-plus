# Which compiler
#CXX=clang++-11
CXX=g++-9
CXXFLAGS += -O3
#CXXFLAGS += -g

# To debug the Boost.Fiber pool executor benchmark
#CXXFLAGS += -DTRISYCL_FIBER_POOL_DEBUG
#CXXFLAGS += -I /home/rkeryell/Xilinx/Projects/C++/Boost/boost-root
#LDFLAGS += -L /home/rkeryell/Xilinx/Projects/C++/Boost/boost-root/stage/lib

#CXXFLAGS += -I ~/Xilinx/Projects/C++/range-v3/include -Wall -std=c++2a -D__cpp_concepts=0
#CXXFLAGS += -Wall -std=c++2a -U__cpp_concepts
#CXXFLAGS += -Wall -std=c++2a
CXXFLAGS += -I ~/Xilinx/Projects/C++/range-v3/include -Wall -std=c++2a
# To use Clang C++ standard library:
#CXXFLAGS += -stdlib=libc++

# To use Boost.Thread & Boost.Fiber
LDLIBS += -lboost_thread -lboost_system -lboost_fiber -lboost_context -lpthread

TARGETS = bin_packing_best_worst_fit \
	Boost/Fiber/benchmark \
	Boost/Fiber/boost_fiber \
	Boost/Fiber/fibers_in_threads \
	Boost/Fiber/fibers_with_threads \
	constexpr_fibonacci \
	meta_iterate \
	move/vectors \
	non-initializing/uninitialized_vector \
	permute_string_letters \
	permute_unique_string_letters \
	quizz/quizz \
	ranges \
	ref_to_array \
	test_parenthesis \
	try_set

all: $(TARGETS)

# Make NTTP_ref from NTTP_ref.cpp and NTTP_ref_2.cpp
NTTP_ref: NTTP_ref_2.o

clean:
	rm -f $(TARGETS)
