CXXFLAGS = -Wall -std=c++2a -O3

TARGETS = bin_packing_best_worst_fit constexpr_fibonacci \
  permute_string_letters \
	permute_unique_string_letters test_parenthesis try_set

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
