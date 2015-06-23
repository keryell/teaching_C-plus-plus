CXXFLAGS = -Wall -std=c++1y -g

TARGETS = bin_packing_best_worst_fit permute_string_letters \
	 test_parenthesis try_set

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
