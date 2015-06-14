CXXFLAGS = -Wall -std=c++1y -g

TARGETS = bin_packing_best_worst_fit test_parenthesis try_set

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
