CPPFLAGS=-std=c++2a -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

9cc: $(OBJS)
	$(CXX) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): $(wildcard *.h)

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
