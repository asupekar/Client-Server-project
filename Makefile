all: client server

client: client.o
	g++ -o -lpthread client client.o

server: server.o
	g++ -o -lpthread server server.o

client.o: client.cpp
	g++ -c client.cpp

server.o: server.cpp
	g++ -c server.cpp

clean:
	rm *.o client server