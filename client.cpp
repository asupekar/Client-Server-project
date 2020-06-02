// Client side C/C++ program to demonstrate Socket programming
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <poll.h>
#include <stdio.h>

// This port and host will automatically be passed in if client is called without arguments
#define PORT "12126"
#define HOST "127.0.0.1"

using namespace std;

// stores information shared by threads
class GlobalContext {
private:
    bool connected;
    int socket;
    string username;
    pthread_mutex_t lock;
    string chatPartner;
    bool chatMode;
public:
    //Constructor
    GlobalContext() {
        chatPartner = "";
    }

    void setConnect(bool connected)
    {
        this->connected = connected;
    }

    void setUser(string username)
    {
        this->username = username;
    }

    void setSocket(int socket)
    {
        this->socket = socket;
    }

    void setChatPartner(string username)
    {
        this->chatPartner = username;
    }

    void setChatMode(bool mode)
    {
        this->chatMode = mode;
    }

    string getUser()
    {
        return username;
    }

    bool getConnect()
    {
        return connected;
    }

    int getSocket()
    {
        return socket;
    }

    string getChatPartner() {
        return chatPartner;
    }

    bool getChatMode() {
        return chatMode;
    }

    pthread_mutex_t * getLock() {
        return &lock;
    }
};

// making a basic error handler to get rid of printing the buffer
// used in: disconnect rpc, sendmessage rpc
// assumes success = 1
// assumes the first line of buffer will always be status = int
int basicErrorHandling(char buffer[]) {
    char * status = strtok(buffer, ";=");
    int i = 0;
    int result = 0;
    while (status != NULL) {
        // second one is status integer
        if (i == 1) {
            result = atoi(status);
        } else if (result == 1) {
            //if result is good (1)
            //then print no more
            return result;
        } else if (i > 1) {
            //print the rest
            printf("%s\n", status);
        }
        i++;
        status = strtok(NULL,";=");
    }
    return result;
}

// Making the initial connection to the server
int connectToServer(char *szHostName, char *szPort, int & sock)
{
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    auto port = (uint16_t) atoi(szPort);
    serv_addr.sin_port = htons(port);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Socket creation error" << endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0)
    {
        cout << "Invalid address/ Address not supported" << endl;
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Connection Failed" << endl;
        return -1;
    }
    return 0;
}

// Client side disconnect RPC
// Sends "rpc=disconnect;" to the server for processing and receives a buffer back
// Disconnect is then processed
void disconnectRPC(int & sock) {
    char disconnectCall[256];
    strcpy(disconnectCall,"rpc=disconnect;");
//    cout << "Passing in: " << disconnectCall << endl;
//    cout << "String length being passed in: " << strlen(disconnectCall) << endl;
    send(sock, disconnectCall, strlen(disconnectCall)+1, 0);

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="status=<errorStatus>;error=<errorMessage>
    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
    //printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    int out = basicErrorHandling(buffer);
    close(sock);
}

// Parses buffer into a vector
// Parsed buffer vector will store in format [status, errorStatus, error, errorMessage]
vector<string> getStatusorError(string buffer) {
//    cout << "Buffer being passed in is " << buffer << endl;
    vector<string> vec;
    string::size_type pos;
    while((pos = buffer.find_first_of("=;")) != string::npos){
        string str = buffer.substr(0, pos);
        vec.push_back(str);
        buffer.erase(0, pos+1);
    }
    return vec;
}

// Client side connect RPC
// Asks user for username and password and sends connect request to server
// Prints out server's response
string connectRPC(int & sock, string & username, string & password)
{
    // Input Arguments are: username, password
    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    // Formulates input to send to server
    // input format="rpc=connect;username=<Your user>;password=<Your password>;"
    char authStr[256];
    strcpy(authStr,"rpc=connect;username=");
    strcat(authStr, username.c_str());
    strcat(authStr, ";password=");
    strcat(authStr, password.c_str());
    strcat(authStr, ";");
    send(sock, authStr, strlen(authStr)+1, 0);
    cout << "Sent login details, waiting for server response..." << endl;

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="status=<errorStatus>;error=<errorMessage>"
    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
    //printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    // returns entire buffer, to be parsed later
    return buffer;
}

// Client side send message RPC
// changed inputs to take in thread info
int sendMessage(GlobalContext * clientData, bool prompt, string message="", string msgFromUser="") {
    //read globalcontextdata
    int sock = clientData->getSocket();
    string fromUser = clientData->getUser();

    // start buffer
    string toUser;
    int buffersize = 1024;
    char sendBuffer[buffersize];
    strcpy(sendBuffer, "rpc=sendmessage;toUser=");
    // prompt
    if (prompt == true)
    {
        // get user
        if (clientData->getChatPartner().empty()) {
            printf("Who would you like to send a message to? ");
            if (cin.peek() !='\0') {
                cin.ignore();
            }
            getline(cin, toUser, '\n');
            clientData->setChatPartner(toUser);
            clientData->setChatMode(true);
        } else {
            toUser = clientData->getChatPartner();
        }
        // get message
        strcat(sendBuffer, toUser.c_str());
        strcat(sendBuffer, ";fromUser=");
        strcat(sendBuffer, fromUser.c_str());
        strcat(sendBuffer, ";message=");
        printf("Message: ");
        getline(cin, message, '\n');
        strcat(sendBuffer, message.c_str());
    } else {
//        toUser = clientData->getChatPartner();
        strcat(sendBuffer, msgFromUser.c_str());
        strcat(sendBuffer, ";fromUser=");
        strcat(sendBuffer, fromUser.c_str());
        strcat(sendBuffer, ";message=");
        strcat(sendBuffer, message.c_str());
    }

    //end the line
    char temp[2] = {';'};
    strcat(sendBuffer,temp);

    //send
    if (message[0] != '\0'){
        //cout << sendBuffer << endl;
        send(sock, sendBuffer, strlen(sendBuffer)+1, 0);

        //read response
        char * buffer = new char[buffersize];
        read(sock, buffer, 1024);
        //cout << buffer << endl;
        int out = basicErrorHandling(buffer);

        //if it didn't return a success
        //cout << out << endl;
        if (out != 1) {
            //exit chat
            clientData->setChatPartner("");
            clientData->setChatMode(false);
        }

        return out;
    }
    return 0;

    // Check if it sent correctly
    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="Message successfully sent to <username>

/*
    char *output = NULL;
    char substring1[] = "offline at this moment";
    output = strstr (buffer,substring1);
    if(output) {
        return -1;
    }
    char substring2[] = "SetAwayMessage=";
    output = strstr (buffer,substring2);
    if(output) {
        return -2;
    }
    return 0;*/
}

// Client side check online user RPC
string checkOnlineUsers(int & sock) {
    char authStr[64];
    strcpy(authStr, "rpc=checkonlineusers;");
    send(sock, authStr, strlen(authStr)+1, 0);

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="status=<errorStatus>;error=<errorMessage>"
    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
    //printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    // returns entire buffer, to be parsed later
    char* printit;
    printit = strtok(buffer, ",=");
    int i = 0;
    while (printit != NULL) {
        //skip status message
        if (i > 2) {
            printf("Online: %s\n", printit);
        }
        i++;
        printit = strtok(NULL,",=");
    }
    return buffer;
}

// Client side set away message RPC
string setAwayMessage(int & sock) {
    char authStr[1024];
    string awayMessage;

    cout << "What away message would you like to set? ";
    cin.ignore();
    getline(cin, awayMessage, '\n');

    strcpy(authStr, "rpc=setaway;awayMessage=");
    strcat(authStr, awayMessage.c_str());
    strcat(authStr, ";");
    send(sock, authStr, strlen(authStr)+1, 0);

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="status=<errorStatus>;error=<errorMessage>"
    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
    //printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    printf("%s\n", buffer);
    // returns entire buffer, to be parsed later
    return buffer;
}

bool helpMessage() {
    cout << "--------------------" << endl;
    cout << "\nAvailable commands: " << endl;
    cout << "1. Chat" << endl;
    cout << "2. Check Online Users (Check)" << endl;
    cout << "3. Set Away Message (Away)" << endl;
    cout << "4. Disconnect "<<endl;
    cout << "Any other number. Just to be online and be a good listener" << endl;
    cout << "--------------------" << endl;
    return true;
}

// Initial parse of the RPC call. Parses by ; to separate out all of the arguments
// End result will be a vector of format: [rpc, rpcType, parameter1, parameter1value, etc...]
static string parseFromUser(string input) {
    //cout << "parseFromuser:: " << input << endl;
    vector<string> vec;
    string::size_type pos;
    string val;
    while((pos = input.find_first_of(';')) != string::npos){
        string str = input.substr(0, pos);
        vec.push_back(str);
        input.erase(0, pos+1);
    }

    string s = vec.front();
    while((pos = s.find_first_of('=')) != string::npos){
        string key = s.substr(0, pos);
        s.erase(0, pos+1);
        val = s.substr(0, s.length());
    }

    return val;
}

static string parseMessage(string input) {
    // cout << "parseFromMessage:: " << input << endl;
    vector<string> vec;
    string::size_type pos;
    string val;
    while((pos = input.find_first_of(';')) != string::npos){
        string str = input.substr(0, pos);
        vec.push_back(str);
        input.erase(0, pos+1);
    }

    string s = vec.back();
    while((pos = s.find_first_of('=')) != string::npos){
        string key = s.substr(0, pos);
        s.erase(0, pos+1);
        val = s.substr(0, s.length());
    }

    return val;
}

// after you get an unknown incoming message
// decide whether or not to accept chat
// delete or comment out if not using threads
string acceptChat(GlobalContext * data, string fromUser) {
    string acceptChat;
    printf("Do you want to enter chat with %s? yes or no: ",fromUser.c_str());

    getline(cin,acceptChat,'\n');
    if (acceptChat.compare("yes") == 0) {
        // if you're already in chat, tell them you're done
        printf("Accepted!\n");
        if (data->getChatMode()) {
            printf("Ending chat with %s\n",data->getChatPartner().c_str());
            sendMessage(data,false,"End Chat");
        } else {
            data->setChatMode(true);
        }
        // enter one-on-one mode
        printf("Starting chat with %s\n",fromUser.c_str());
        data->setChatPartner(fromUser);
    } else {
        //rejected
        data->setChatPartner(fromUser);
        sendMessage(data,false,"No Chat");
        data->setChatPartner("");
    }
    return acceptChat;
}


// one-on-one chat
// delete or comment out if not using threads
static void * chatThread(void * input) {
    GlobalContext * data = (GlobalContext *) input;
    while (true) {
        if (data->getChatMode()) {
            // in chat mode
            string message;
            cout << ">>";
            getline(cin, message, '\n');
            sendMessage(data, false, message, data->getChatPartner());
            if (message.compare("End Chat") == 0) {
                cout << "Chat ending" << endl;
                data->setChatMode(false);
                //data->setChatPartner("");
                pthread_exit(NULL);
            }
            usleep(100000);
        } else {
            pthread_exit(NULL);
        }
    }
    return NULL;
}


// this handles incoming messages from other clients
// delete or comment out if not using threads
static void * readThread(void * input) {
    // cast input to correct type
    GlobalContext * data = (GlobalContext *) input;

    //continuously loop to check for new incoming messages
    while (true) {
        // validate we're still connected
        //if (data->getChatMode()) {
        if (data->getChatMode()) {
            //yes, connected

            //lock the socket while reading
            pthread_mutex_lock(data->getLock());

            // poll the socket
            struct pollfd fd;
            int pollResult;
            fd.fd = data->getSocket(); // your socket handler
            fd.events = POLLIN;
            pollResult = poll(&fd, 1, 500); // .5 second for timeout

            if (pollResult > 0) {
                //read
                char buffer[1024] = {0};
                read(data->getSocket(), buffer, 1024);
                //pthread_mutex_unlock(data->getLock());

                //parse it
                string fromUser = parseFromUser(buffer);
                string message = parseMessage(buffer);
                if(message.length() > 0){
                    printf("[%s]: %s\n", fromUser.c_str(), message.c_str());
                }

                // depending on the message, take further action
                // first check if it's from the current partner
                if (data->getChatPartner().compare(fromUser) != 0) {
                    // it's not the current partner, so prompt
                    sendMessage(data,false,"I'm busy",fromUser);
                }

                // if the message was to leave chat, then exit
                if (message.compare("End Chat") ==  0) {
                    cout << "Leaving one-on-one chat with " << fromUser << endl;
                    //data->setChatPartner("");
                    data->setChatMode(false);
                    pthread_exit(NULL);
                    //pthread_cancel(chatLoop);
                    //pthread_join(chatLoop, NULL);
                    //pthread_create(&sendLoop, NULL, sendThread, (void *) data);
                    // if your request was rejected
                } else if (message.compare("I'm busy") == 0) {
                    cout << fromUser << " did not accept your chat" << endl;
                    //data->setChatPartner("");
                    data->setChatMode(false);
                    pthread_exit(NULL);
                    //pthread_cancel(chatLoop);
                    //pthread_join(chatLoop, NULL);
                    //pthread_create(&sendLoop, NULL, sendThread, (void *) data);
                }
            }

            // done reading, release
            pthread_mutex_unlock(data->getLock());
        } else {
            // client chose to disconnect, stop reading
            pthread_exit(NULL);
            break;
        }
        // allow a wait for commands
        //cout << "pause" << endl;
        usleep(500000);
    }
    return 0;
}


// this handles messages from the client to the server
//void * sendThread(void * input) {
void sendLoop(GlobalContext * data) {
    // cast input to correct type
    // needed if this is a thread
    //GlobalContext * data = (GlobalContext *) input;

    // pull socket to variable for convenience
    int sock = data->getSocket();

    // create strings for users for convenience
    string username = data->getUser();

    // give help first
    int userCommand = 5;
    pthread_t readLoop;
    pthread_t chatLoop;

    // loop over steps
    // lock socket on every call for safety
    while (true) {
        //fix commenting on usercommand==1 to sremove threading
        if (userCommand == 1) {
            cout << "chat partner: ";
            string partner;
            cin >> partner;
            data->setChatPartner(partner);
            data->setChatMode(true);

            pthread_create(&readLoop, NULL, readThread, (void *) data);
            pthread_create(&chatLoop, NULL, chatThread, (void *) data);

            pthread_join(readLoop, NULL);
            pthread_join(chatLoop, NULL);
            cout << "threads ended" << endl;

            data->setChatPartner("");
            data->setChatMode(false);
            cout << "ended" << endl;
            cout << "what next? 5-help " << endl;
            cin >> userCommand;
            /*int ret = sendMessage(data, true);
            if(ret == -1 || ret == -2) {
                userCommand = 4;
            }else{
                userCommand = 6;
            }*/
        } else if (userCommand == 2) {
            checkOnlineUsers(sock);
            cout << "what next? 5-help " << endl;
            cin >> userCommand;
        } else if(userCommand == 3){
            setAwayMessage(sock);
            cout << "what next? 5-help " << endl;
            cin >> userCommand;
        } else if(userCommand == 4){
            data->setConnect(false);
            //wait to give time for the readthread to close
            usleep(500500);
            disconnectRPC(sock);
            break;
        } else if (userCommand == 5) {
            helpMessage();
            cout << "what next? 5-help " << endl;
            cin >> userCommand;
        } else {
            char buffer[1024] = {0};
            while(read(sock, buffer, 1024)){
                string fromUser = parseFromUser(buffer);
                string message = parseMessage(buffer);
                if(message.length() > 0 && (message.compare("SetAway")!=0)){
                    printf("%s: %s\n", fromUser.c_str(), message.c_str());
                    sendMessage(data, true);
                    userCommand = 6;
                }else{
                    cout << "Disconnecting since away..."<< endl;
                    userCommand = 4;
                }
                break;
            }
        }
    }

    //return NULL;
}


int main(int argc, char const *argv[])
{
    int sock = 0;
    GlobalContext * data = new GlobalContext();
    string username;
    string password;
    int connection;

    // Client can input host name and port # as arguments when running the program
    // If the client does not provide these, the global HOST and PORT are used to connect
    if(argc < 2) {
//        cout << "This needs 2 arguments Host name (127.0.0.1) and port #" << endl;
        connection = connectToServer((char *) HOST, (char *) PORT, sock);
    } else {
        connection = connectToServer((char *) argv[1], (char *) argv[2], sock);
    }

    if (connection == -1) {
        cout << "Connection failed, try starting with different parameters" << endl;
        return 0;
    }
    data->setSocket(sock);

    bool connected = false;
    // Continue to ask for user credentials until they are correct
    do {
        string response = connectRPC(sock, username, password);
        vector<string> parsedResponse = getStatusorError(response);
        if (parsedResponse[1] == "1") {
            cout << "Successfully connected!" << endl;
            connected = true;
        } else {
            cout << "The error was a " << parsedResponse[3] << endl;
        }
    } while(!connected);

    // populate the fields
    data->setConnect(connected);
    data->setUser(username);

    // run the authenticated client
    sendLoop(data);
    return 0;
}
/*
int main(int argc, char const *argv[])
{
    int sock = 0;
    string username;
    string password;
    string toUser;
    // Client can input host name and port # as arguments when running the program
    // If the client does not provide these, the global HOST and PORT are used to connect
    if(argc < 2) {
//        cout << "This needs 2 arguments Host name (127.0.0.1) and port #" << endl;
        connectToServer((char *) HOST, (char *) PORT, sock);
    } else {
        connectToServer((char *) argv[1], (char *) argv[2], sock);
    }
    bool connected = false;
    // Continue to ask for user credentials until they are correct
    do {
        string response = connectRPC(sock, username, password);
        vector<string> parsedResponse = getStatusorError(response);
        if (parsedResponse[1] == "1") {
            cout << "Successfully connected!" << endl;
            connected = true;
        } else {
            cout << "The error was a " << parsedResponse[3] << endl;
        }
    } while(!connected);
//    cout << "Waiting for 10 seconds....." << endl;
    // Wait 10 seconds and then disconnect
//    usleep(10000000);

    int userCommand;
    cout << "what next? 5-help " << endl;
    cin >> userCommand;
    while (true) {
        if (userCommand == 1) {
            int ret = sendMessage(sock, username, toUser, true);
            if(ret == -1 || ret == -2) {
                userCommand = 4;
            }else{
                userCommand = 6;
            }
        } else if (userCommand == 2) {
            checkOnlineUsers(sock);
            userCommand = 5;
        } else if(userCommand == 3){
            setAwayMessage(sock);
            userCommand = 4;
        } else if(userCommand == 4){
            disconnectRPC(sock);
            break;
        } else if(userCommand == 5) {
            helpMessage();
            cout << "what next? 5-help " << endl;
            cin >> userCommand;
        } else {
            char buffer[1024] = {0};
            while(read(sock, buffer, 1024)){
                string fromUser = parseFromUser(buffer);
                string message = parseMessage(buffer);
                if(message.length() > 0 && (message.compare("SetAway")!=0)){
                    printf("%s: %s\n", fromUser.c_str(), message.c_str());
                    sendMessage(sock, username, fromUser, false);
                    userCommand = 6;
                }else{
                    cout << "Disconnecting since away..."<< endl;
                    userCommand = 4;
                }
                break;
            }   
        }
    }
    
    return 0;
}
*/