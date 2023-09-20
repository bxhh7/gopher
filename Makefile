CC=gcc
gopher: main.o socket.o types.o
	cc -Wall -Wextra main.o socket.o types.o -o gopher
	rm *.o
	mkdir -p /var/gopher
