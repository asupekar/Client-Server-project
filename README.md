# Client-Server-project
Final group project for CPSC 5042, Spring Quarter

Assumptions:
1. SendMessageRPC can only send and receive messages between two clients. When Client A wants to send message to Client B and Client B is offline, Client A should receive message - "Client B is offline at this moment. Please try again later."
2. Once the SetAwayMessage is set for a client A, any other client who wants to chat with Client A will see the SetAwayMessage and won't be able to chat with client A. It's client A's responsibility to remove the SetAwayMessage first in order to receive chat messages from other clients.
3. If a client A has SetAwayMessage set, then 
    3.1 Client A should not be able to send message to any client
    3.2 When other Client B tries sending message to Client A, it should see SetAwayMessage irrespective of whether Client A is online or not
    

To compile the server and client
1. Copy all files to a local directory
2. either type in `make `
3. Or run the commands below

```
g++ -std=c++11 client.cpp -o -lpthread client.o [On Mac machine]
g++ -Wall -Werror -pedantic -std=c++11 -o client.o -lpthread client.cpp
g++ -o client.o -lpthread client.cpp [On cs1 terminal]

./client.o 127.0.0.1 12123

g++ -Wall -Werror -pedantic -std=c++11 -o server.o -lpthread server.cpp [On Mac machine]

```
After compiling, to start the server, run `./server.o 12123`.
To start the client, run `./client.o`
```
