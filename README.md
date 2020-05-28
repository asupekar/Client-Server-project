# Client-Server-project
Final group project for CPSC 5042, Spring Quarter

To compile the server and client
1. Copy all files to a local directory
2. either type in `make `
3. Or run the commands below

```
g++ -std=c++11 client.cpp -o -lpthread client.o [On Mac machine]
g++ -Wall -Werror -pedantic -std=c++11 -o client.o -lpthread client.cpp
g++ -o client.o -lpthread client.cpp [On cs1 terminal]
g++ -Wall -Werror -pedantic -std=c++11 -o server.o -lpthread server.cpp [On Mac machine]

```
After compiling, to start the server, run `./server.o 12123`.
To start the client, run `./client.o`
```
