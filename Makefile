CXXFLAGS = -Wall -std=c++1y -g

TARGETS = test_parenthesis

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
