# Client-Server-project

g++ -std=c++11 client.cpp -o -lpthread client.o [On Mac machine]


g++ -Wall -Werror -pedantic -std=c++11 -o client.o -lpthread client.cpp

g++ -o client.o -lpthread client.cpp [On cs1 terminal]
./client.o 127.0.0.1 12123

g++ -Wall -Werror -pedantic -std=c++11 -o server.o -lpthread server.cpp [On Mac machine]
./server.o 12123

