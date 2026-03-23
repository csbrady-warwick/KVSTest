CXX := g++
CXXFLAGS := -O3

TARGETS := generate generateOrdered traverse elementAccess

all: $(TARGETS)

generate: generate.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

generateOrdered: generateOrdered.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

traverse: traverse.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

elementAccess: elementAccess.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean