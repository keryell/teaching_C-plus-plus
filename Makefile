CXXFLAGS = -Wall -std=c++2a -O3

# To use Boost.Thread & Boost.Fiber
LDLIBS += -lboost_thread -lboost_system -lboost_fiber -lboost_context

TARGETS = bin_packing_best_worst_fit \
	boost_fiber \
	constexpr_fibonacci \
	meta_iterate \
	permute_string_letters \
	permute_unique_string_letters \
	ranges \
	test_parenthesis try_set \
	uninitialized_vector

all: $(TARGETS)

# Make NTTP_ref from NTTP_ref.cpp and NTTP_ref_2.cpp
NTTP_ref: NTTP_ref_2.o

clean:
	rm -f $(TARGETS)
