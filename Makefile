9cc: compiler.cpp
	g++ -std=c++2a -static -o 9cc compiler.cpp

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
