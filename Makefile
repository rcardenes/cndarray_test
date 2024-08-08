CXXFLAGS=-std=c++17 -I$(HOME)/.local/include -L$(HOME)/.local/lib -L$(HOME)/.local/lib64 -pthread
LDFLAGS=-lcnpy -lredis++
TARGETS=producer consumer

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TARGETS)
