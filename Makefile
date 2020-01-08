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

clean:
	rm -f $(TARGETS)
