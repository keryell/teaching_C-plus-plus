# Just try make on a Unix-like system

# Adapt to the require compiler
#CXX=clang++-11
CXX=g++-9
# To use Clang C++ standard library:
#CXXFLAGS += -stdlib=libc++

# Warnings are useful
CXXFLAGS += -Wall

# Which version of C++
CXXFLAGS += -std=c++2a

# To optimize or to debug... That is the question
#CXXFLAGS += -O3
CXXFLAGS += -g

# To debug the Boost.Fiber pool executor benchmark
#CXXFLAGS += -DTRISYCL_FIBER_POOL_DEBUG
#CXXFLAGS += -I /home/rkeryell/Xilinx/Projects/C++/Boost/boost-root
#LDFLAGS += -L /home/rkeryell/Xilinx/Projects/C++/Boost/boost-root/stage/lib

#CXXFLAGS += -I ~/Xilinx/Projects/C++/range-v3/include -Wall -std=c++2a -D__cpp_concepts=0
#CXXFLAGS += -Wall -std=c++2a -U__cpp_concepts
#CXXFLAGS += -I ~/Xilinx/Projects/C++/range-v3/include

# To use Boost.Thread & Boost.Fiber
LDLIBS += -lboost_thread -lboost_system -lboost_fiber -lboost_context -lpthread

TARGETS = \
	Boost/Fiber/benchmark \
	Boost/Fiber/boost_fiber \
	Boost/Fiber/fibers_in_threads \
	Boost/Fiber/fibers_with_threads \
	constexpr/constexpr_fibonacci \
	meta-programming/meta_iterate \
	move/vectors \
	non-initializing/remove_initialization \
	non-initializing/uninitialized_vector \
	NTTP/NTTP_ref \
	obsolete_to_clean_up/bin_packing_best_worst_fit \
	obsolete_to_clean_up/permute_string_letters \
	obsolete_to_clean_up/permute_unique_string_letters \
	obsolete_to_clean_up/test_parenthesis \
	obsolete_to_clean_up/try_set \
	quizz/quizz \
	ranges/ranges \
	ref/ref_to_array

all: $(TARGETS)

# Make NTTP_ref from NTTP_ref.cpp and NTTP_ref_2.cpp
NTTP/NTTP_ref: NTTP/NTTP_ref_2.o

clean:
	rm -f $(TARGETS)
