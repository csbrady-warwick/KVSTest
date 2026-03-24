CXX := g++
CXXFLAGS := -O3 --std=c++17

TARGETS := bin/generate bin/generateOrdered bin/traverse bin/elementAccess

all: $(TARGETS)

bin/generate: src/generate.cpp | dirs
	$(CXX) $(CXXFLAGS) -o $@ $<

bin/generateOrdered: src/generateOrdered.cpp | dirs
	$(CXX) $(CXXFLAGS) -o $@ $<

bin/traverse: src/traverse.cpp | dirs
	$(CXX) $(CXXFLAGS) -o $@ $<

bin/elementAccess: src/elementAccess.cpp | dirs
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf output
	rm -rf bin

dirs:
	mkdir -p bin
	mkdir -p output

.PHONY: all clean dirs