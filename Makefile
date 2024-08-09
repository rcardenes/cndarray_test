CXXFLAGS=-std=c++20 -I$(HOME)/.local/include -L$(HOME)/.local/lib -L$(HOME)/.local/lib64 -pthread
LDFLAGS=-lcnpy -lhiredis -lredis++
TARGETS=producer consumer

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TARGETS)
