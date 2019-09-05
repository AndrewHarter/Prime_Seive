sieve: sieve.cpp socket.o
	g++ -std=c++11 socket.o sieve.cpp -o sieve

socket.o: socket.cpp socket.h
	g++ socket.cpp -c

clean:
	rm *.o sieve

