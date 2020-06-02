# Client-Server-project
Final group project for CPSC 5042, Spring Quarter

Assumptions:
<ol>
    <li>When users begin chatting, they must set a primary chat partner. 
        <ol>
            <li>Unless noted otherwise, messages are delivered to the chat partner.</li>
            <li>If Client A is chatting with Client B and Client C messages Client B, Client C will automatically receive a response that Client B is alredy in chat and may not respond.</li>
        </ol>
    </li>
    <li>When Client A wants to send message to Client B and Client B is offline, Client A should receive message - "Client B is offline at this moment. Please try again later."</li>
    <li>Once the SetAwayMessage is set for a client A, any other client who wants to chat with Client A will see the SetAwayMessage and won't be able to chat with client A. It's client A's responsibility to remove the SetAwayMessage first in order to receive chat messages from other clients.</li>
    <li>If a client A has SetAwayMessage set, then
        <ol>
            <li>Client A should not be able to send message to any client</li>
            <li>When other Client B tries sending message to Client A, it should see SetAwayMessage irrespective of whether Client A is online or not
        </ol>
    </li>
    <li>In order to end a chat or change chat partner, one of the users must use the /endchat command to end the chat.</li>
    <li>If the server is exited, the user away messages are not stored. All users are online when they log in initially.</li>
    <li>There are only credentials for 4 users available, but more can be added to the userInfo.csv if desired. For simplicity please copy the same password as the other "accounts" as there is backend password decryption happening.</li>
</ol>

To compile the server and client
1. Copy all files to a local directory
2. either type in `make`
3. Or run the commands below

```
g++ -std=c++11 client.cpp -o -lpthread client.o [On Mac machine]
g++ -Wall -Werror -pedantic -std=c++11 -o client.o -lpthread client.cpp
g++ -o client.o -lpthread client.cpp [On cs1 terminal]

g++ -Wall -Werror -pedantic -std=c++11 -o server.o -lpthread server.cpp [On Mac machine]

```
After compiling, to start the server, run `./server 12123`.
To start the client, run `./client`

Sample run to test all functionality:
<ul>
    <li>Launch 1 server instance using ./server PORT </li>
    <li>Launch multiple client instances using ./client </li>
    <li>Log in with the following credentials:
        <ul>
            <li>Ruifeng : password123</li>
            <li>Naomi : password123</li>
            <li>Aish : password123</li>
            <li>Mike: password123</li>
        </ul>
    </li>
    <li>On any client, press 2 to get a list of all online users.</li>
    <li>Disconnect any client and then check the list of online users to see that user is no longer online.</li>
    <li>On any client (Client A), press 3 and then set a custom away message. Now this user will not be able to be messaged.
        <ul>
            <li>On Client B, press 1 to start a chat.</li>
            <li>Type in the username of Client A.</li>
            <li>Type a message and send it to Client A to receive the away message that Client A set previously.</li>
            <li>Now, on Client A< press 6 to return from being away.</li>
            <li>On Client B, press 1 again to start a chat.</li>
            <li>Type in the username of Client A.</li>
            <li>Type a message and send it to Client A, and now you can see you will be able to successfully have a conversation between Clients A and B.</li>
        </ul>
    </li>
    <li>On any client (Client A), press 1 to start a chat. 
        <ul>
            <li>Enter the username of the client (Client B) you wish to chat with.</li>
            <li>Type the message you wish to send to Client B.</li>
            <li>On Client B, press 1 as well and enter the username of Client A.</li>
            <li>The chat has now begun.</li>
            <li>Chat back and forth to see the messages.</li>
            <li>Now, on Client C, press 1 and enter the username of Client B.</li>
            <li>Client C will automatically receive a message that Client B is busy and the chat will end.</li>
            <li>To end the chat between Client A and Client B, type "End Chat" on whichever Client currently is sending.</li>
            <li>Now the chat is over and both clients will be taken to the main menu.</li>
        </ul>
    </li>
</ul>


