9cc: compiler.cpp compile.cpp
	g++ -std=c++2a -static -o 9cc compiler.cpp compile.cpp

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
