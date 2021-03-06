// Client side C/C++ program to demonstrate Socket programming
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <poll.h>
#include <stdio.h>
#include <limits>

// This port and host will automatically be passed in if client is called without arguments

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

    // Sets if a user is connected
    void setConnect(bool connected) {
        this->connected = connected;
    }

    // Sets the associated username
    void setUser(string username) {
        this->username = username;
    }

    // Sets the associated socket
    void setSocket(int socket) {
        this->socket = socket;
    }

    // Sets the current chat partner for this user
    void setChatPartner(string username) {
        this->chatPartner = username;
    }

    // Sets the chatmode for this user (true / false)
    void setChatMode(bool mode) {
        this->chatMode = mode;
    }

    // Returns the user associated
    string getUser() {
        return username;
    }

    // Returns if connected
    bool getConnect() {
        return connected;
    }

    // Returns the socket
    int getSocket() {
        return socket;
    }

    // Returns the current chat partner
    string getChatPartner() {
        return chatPartner;
    }

    // Returns the current chat mode
    bool getChatMode() {
        return chatMode;
    }

    // returns the mutex lock
    pthread_mutex_t * getLock() {
        return &lock;
    }
};

// This function could be commented out, not in use anymore
// making a basic error handler to get rid of printing the buffer
// used in: disconnect rpc, sendmessage rpc
// assumes success = 1
// assumes the first line of buffer will always be status = int
// NOTE: bug - if there are any pending messages, it will print the first one
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
int connectToServer(char *szHostName, char *szPort, int & sock) {
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    auto port = (uint16_t) atoi(szPort);
    serv_addr.sin_port = htons(port);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error" << endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported" << endl;
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
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
//    printf("ValRead=%zu buffer=%s\n", valRead, buffer);
//    int out = basicErrorHandling(buffer);
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
//    printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    // returns entire buffer, to be parsed later
    return buffer;
}


// Client side send message RPC
// Changed inputs to take in thread information
// Since the prompting is now in thread loops, changed the inputs
// input: clientData = thread info
// input: message = message to deliver
// input: toUser = optional, person to deliver to
void sendMessage(GlobalContext * clientData, string message, string toUser="") {
    // start buffer
    int buffersize = 1024;
    char sendBuffer[buffersize];
    strcpy(sendBuffer, "rpc=sendmessage;toUser=");

    // if they didn't fill in the optional toUser field
    // then get the value from the current chatPartner 
    if (toUser.empty())
    {
        // get user, if it's empty, then prompt
        if (clientData->getChatPartner().empty()) {
            printf("Who would you like to chat with? ");
            if (cin.peek() =='\n') {
                cin.ignore();
            }
            getline(cin, toUser, '\n');

            // then set this as the persistent chat partner
            clientData->setChatPartner(toUser);
            clientData->setChatMode(true);
        } else {
            toUser = clientData->getChatPartner();
        }
    }

    // add toUser to buffer
    strcat(sendBuffer, toUser.c_str());

    // add current user to buffer
    strcat(sendBuffer, ";fromUser=");
    strcat(sendBuffer, clientData->getUser().c_str());

    // add message to buffer
    strcat(sendBuffer, ";message=");
    strcat(sendBuffer, message.c_str());

    //end the line
    char temp[2] = {';'};
    strcat(sendBuffer,temp);

    //send
    if (message[0] != '\0'){
        //cout << sendBuffer << endl;
        send(clientData->getSocket(), sendBuffer, strlen(sendBuffer)+1, 0);

        // read response
        char * buffer = new char[buffersize];
        read(clientData->getSocket(), buffer, 1024);

        // print errors if there were any
//        int out = basicErrorHandling(buffer);
        vector<string> parsedResponse = getStatusorError(buffer);
        //if it didn't return a success
        if (parsedResponse[1] != "1") {
            // Checks for specific error message
            if (parsedResponse[3] == "YouAreAway") {
                cout << "You are currently away, please return from being away before chatting." << endl;
            } else if(parsedResponse[3] == "BadUsername") {
                cout << "This user does not exist." << endl;
            } else if(parsedResponse[3] == "NotOnline") {
                cout << "The user you are trying to reach is offline." << endl;
            } else if(parsedResponse[3] == "Away") {
                cout << "The user is away" << endl;
                cout << "Their away message says: " << parsedResponse[5] << endl;
            }
//            cout << "There was an error sending your message to " << toUser << endl;

            // if this is the main chatPartner, close out
            if (toUser.compare(clientData->getChatPartner()) == 0) {
                // exiting chat
                cout << "Exiting chat" << endl;
                clientData->setChatPartner("");
                clientData->setChatMode(false);
            }
        }
//        return out;
    }
//    return 0;
}

// Client side check online user RPC
string checkOnlineUsers(int & sock) {
    char authStr[64];
    strcpy(authStr, "rpc=checkonlineusers;");
    send(sock, authStr, strlen(authStr)+1, 0);

    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);

    // from the buffer, just read the users and list them as online
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
void setAwayMessage(int & sock) {
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
//     printf("ValRead=%zu buffer=%s\n", valRead, buffer);
//     printf("%s\n", buffer);

    vector<string> parsedResponse = getStatusorError(buffer);
    if (parsedResponse[1] == "1") {
        cout << parsedResponse[3] << endl;
    }
}

// Client side checking user's away message
void checkAwayMessage(int & sock) {
    char returnCall[256];
    strcpy(returnCall,"rpc=checkAwayMessage;");
    send(sock, returnCall, strlen(returnCall)+1, 0);
    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    vector<string> parsedResponse = getStatusorError(buffer);
    // Prints user's current away message
    if (parsedResponse[1] == "1" && parsedResponse[3] == "You currently have no away message set") {
        cout << parsedResponse[3] << endl;
    } else {
        cout << "Your current away message is: " << parsedResponse[3] << endl;
    }
}

// Client side to allow a user to return from being away
void returnFromAway(int & sock) {
    char returnCall[256];
    strcpy(returnCall,"rpc=returnFromAway;");
//    cout << "Passing in: " << disconnectCall << endl;
//    cout << "String length being passed in: " << strlen(disconnectCall) << endl;
    send(sock, returnCall, strlen(returnCall)+1, 0);

    size_t valRead;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
//    printf("ValRead=%zu buffer=%s\n", valRead, buffer);

    // Parses the buffer returned from the server
    vector<string> parsedResponse = getStatusorError(buffer);

    // Prints out the response from server
    if (parsedResponse[1] == "1") {
        cout << parsedResponse[3] << endl;
    }
}

//  Allows user to print a menu and see what commands are available
bool helpMessage() {
    cout << "--------------------" << endl;
    cout << "Available commands: " << endl;
    cout << "1. Chat" << endl;
    cout << "2. Check Online Users" << endl;
    cout << "3. Set Away Message" << endl;
    cout << "4. Check Away Message" << endl;
    cout << "5. Return from away"<< endl;
    cout << "6. Help message" << endl;
    cout << "7. Disconnect" << endl;
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

// Parsing of the message
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

// This thread manages sending messages in chat
static void * chatThread(void * input) {
    GlobalContext * data = (GlobalContext *) input;

    //loop forever
    while (true) {

        // Check if the thread is in chatMode=true
        if (data->getChatMode()) {

            // if yes, get the next message to send
            string message;
            pthread_mutex_lock(data->getLock());
            printf(">>");
            pthread_mutex_unlock(data->getLock());
            getline(cin, message, '\n');

            // if the message matches a command
            // then take action
            // Case: they sent an empty message
            if (message.empty()) {
                //printf(">>");
            // Case: if they started with an @ symbol
            } else if (message.at(0) == '@') {
                // Collect the name and message
                string toUser = message.substr( 1, message.find(' ') - 1 );
                string parsedMessage = message.substr( message.find(' ') + 1, message.length() - message.find(' '));

                // Send the message to the name
                sendMessage(data, parsedMessage, toUser);

            // Case: they started a / command
            } else if (message.at(0) == '/') {
                // Collect the command
                string command = message.substr(
                    1, 
                    message.find(' ') - 1
                );

                // take action based on the command
                // Case: command is endchat
                if (command.compare("endchat") == 0) {
                    pthread_mutex_lock(data->getLock());
                    cout << "Chat ending" << endl;
                    cout << "----------" << endl;

                    //Send an end message to the other side if it's one-on-one
                    sendMessage(data, "-- leaving chat", data->getChatPartner()); 

                    // Close out all chat settings and leave thread
                    data->setChatMode(false);
                    data->setChatPartner("");
                    pthread_mutex_unlock(data->getLock());
                    pthread_exit(NULL);

                // Case: we don't know
                } else {
                    printf("Unknown command");
                }

            // Case: no commands, just send the message
            } else {
                sendMessage(data, message, data->getChatPartner());
            }

            usleep(100000);

        // Case: chat mode is false, so leave chat
        } else {
            printf("---------\n");
            pthread_exit(NULL);
        }
    }
    return NULL;
}

// This thread handles incoming messages from other clients
static void * readThread(void * input) {
    // cast input to correct type
    GlobalContext * data = (GlobalContext *) input;

    //continuously loop to check for new incoming messages
    while (true) {
        // validate we're still chatting
        if (data->getChatMode()) {

            //lock the socket while reading
            pthread_mutex_lock(data->getLock());

            // poll the socket
            struct pollfd fd;
            int pollResult;
            fd.fd = data->getSocket(); // your socket handler
            fd.events = POLLIN;
            pollResult = poll(&fd, 1, 500); // .5 second for timeout

            if (pollResult > 0) {
                // read
                char buffer[1024] = {0};
                read(data->getSocket(), buffer, 1024);

                //parse it
                string fromUser = parseFromUser(buffer);
                string message = parseMessage(buffer);
                if(message.length() > 0){
                    printf("[%s]: %s\n>>", fromUser.c_str(), message.c_str());
                }

                // NOTE: changed based on additional / commands added
                // depending on the message, take further action
                // first check if it's from the current partner
                if (data->getChatPartner().compare(fromUser) != 0) {
                    // if it's not an automated response
                    // then give them a heads up
                    if (message.compare("-- leaving chat") != 0 && 
                        message.compare("-- is in chat with someone else") != 0) {
                        sendMessage(data,"-- is in chat with someone else",fromUser);
                    }

                // Case: if the message was to leave chat, then exit the one-on-one
                } else if (message.compare("-- leaving chat") ==  0) {
                    printf("Leaving one-on-one chat with %s. Press enter to continue.\n", fromUser.c_str());

                    //unlock the mutex we held
                    fflush(stdout);
                    pthread_mutex_unlock(data->getLock());
                    
                    //signal to chatThread the status
                    data->setChatPartner("");
                    data->setChatMode(false);

                    //exit
                    pthread_exit(NULL);

                // Case: they're not chatting to you
                // Possibly no longer needed with / commands
                } else if (message.compare("-- is in chat with someone else") == 0) {
                    printf("%s might not reply.\n>>", fromUser.c_str());
                    //data->setChatPartner("");
                    //data->setChatMode(false);
                    //pthread_exit(NULL);
                }
            }

            // done reading, release
            fflush(stdout);
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
void sendLoop(GlobalContext * data) {
    // pull socket to variable for convenience
    int sock = data->getSocket();

    // create strings for users for convenience
    string username = data->getUser();

    // give help first
    int userCommand = 6;
    pthread_t readLoop;
    pthread_t chatLoop;

    // loop over steps
    // lock socket on every call for safety
    while (true) {
        // User chatting
        if (userCommand == 1) {
            cout << "Who would you like to chat with?: ";
            string partner;
            if (cin.peek() == '\n') {
                cin.ignore();
            }
            getline(cin, partner, '\n');
            cout << "Setting your chat partner to " << partner << endl;
            data->setChatPartner(partner);

            cout << "----------" << endl;
            cout << "You may message anyone else by beginning your message with @username" << endl;
            cout << "To exit chat, use the command /endchat" << endl;
            cout << "----------" << endl;

            data->setChatMode(true);
            cout << ">>";

            pthread_create(&readLoop, NULL, readThread, (void *) data);
            pthread_create(&chatLoop, NULL, chatThread, (void *) data);

            pthread_join(readLoop, NULL);
            pthread_join(chatLoop, NULL);
            //cout << "threads ended" << endl;

            data->setChatPartner("");
            data->setChatMode(false);
            //cout << "ended" << endl;
            cout << "What would you like to do? 6 -- help " << endl;
            cin >> userCommand;
        // User checking online users
        } else if (userCommand == 2) {
            checkOnlineUsers(sock);
            cout << "\nWhat would you like to do? 6 -- help " << endl;
            cin >> userCommand;
        // User setting an away message
        } else if(userCommand == 3) {
            setAwayMessage(sock);
            cout << "\nWhat would you like to do? 6 -- help " << endl;
            cin >> userCommand;
        // User checking their away message
        } else if(userCommand == 4) {
            checkAwayMessage(sock);
            cout << "\nWhat would you like to do? 6 -- help " << endl;
            cin >> userCommand;
        // User returning from being away
        } else if(userCommand == 5){
            returnFromAway(sock);
            cout << "\nWhat would you like to do? 6 -- help " << endl;
            cin >> userCommand;
        // User wants to review the help message
        } else if (userCommand == 6) {
            helpMessage();
            cout << "What would you like to do? " << endl;
            cin >> userCommand;
        // User wants to disconnect
        } else if (userCommand == 7) {
            data->setConnect(false);
            // wait to give time for the readthread to close
            cout << "Goodbye " << data->getUser() << endl;
            usleep(500500);
            disconnectRPC(sock);
            break;
        // Invalid number or input entered
        } else {
            if(!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            cout << "Please enter a valid selection!" << endl;
            cout << "What would you like to do? " << endl;
            cin >> userCommand;
        }
    }
}

// Main method for client side
int main(int argc, char const *argv[]) {
    int sock = 0;
    GlobalContext * data = new GlobalContext();
    string username;
    string password;
    int connection;

    // Client can input host name and port # as arguments when running the program
    // If the client does not provide these, the global HOST and PORT are used to connect
    if(argc < 2) {
        cout << "This needs 2 arguments Host name (127.0.0.1) and port #" << endl;
        return -1;
//        connection = connectToServer((char *) HOST, (char *) PORT, sock);
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
            cout << "\nWelcome to the chatroom " << username << endl;
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