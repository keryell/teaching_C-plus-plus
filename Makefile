CXXFLAGS = -Wall -std=c++1y -g

TARGETS = bin_packing_best_worst_fit.cpp test_parenthesis

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
