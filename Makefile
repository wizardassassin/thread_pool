CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wpedantic -Werror -Wextra

EXCALL := $(wildcard *.cpp)
EXCALL := $(EXCALL:.cpp=.out)

.PHONY: all debug release

all: release

debug: CXXFLAGS += -g

release: CXXFLAGS += -Ofast -march=native -flto

all: $(EXCALL)

debug: $(EXCALL)

release: $(EXCALL)

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(EXCALL)
