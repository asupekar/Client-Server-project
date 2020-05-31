// Client side C/C++ program to demonstrate Socket programming
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

// This port and host will automatically be passed in if client is called without arguments
#define PORT "12126"
#define HOST "127.0.0.1"

using namespace std;

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
    printf("ValRead=%zu buffer=%s\n", valRead, buffer);
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
    printf("ValRead=%zu buffer=%s\n", valRead, buffer);
    // returns entire buffer, to be parsed later
    return buffer;
}

// Client side send message RPC
int sendMessage(int & sock, string & fromUser, string & toUser, bool prompt) {
    char authStr[1024];
    string message;

    if (prompt == true)
    {
        cout << "Who would you like to send a message to? ";
        cin >> toUser;
        cin.sync();
        cin.get();
        cout << "What message would you like to send? " << endl;  
    }
    cout << fromUser << ": ";
    //cin.ignore(1,'\n');
    // getchar();
    cin >> message;
    char buffer[1024] = { 0 };

    strcpy(authStr, "rpc=sendmessage;toUser=");
    strcat(authStr, toUser.c_str());
    strcat(authStr, ";fromUser=");
    strcat(authStr, fromUser.c_str());
    strcat(authStr, ";message=");
    strcat(authStr, message.c_str());
    strcat(authStr, ";");
    if(message.length() > 0){
        send(sock, authStr, strlen(authStr)+1, 0);
    }

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      (This will be some sort of message (error or success))
    // output format="Message successfully sent to <username>
    size_t valRead;
    valRead = read(sock, buffer, 1024);
    // Printing out the valRead and the buffer for validation purposes
    // cout << "First character is " << buffer[0] << endl;
    //cout << "Message received " << endl;
    printf("%s\n", buffer);
    
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
    return 0;
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
    printf("ValRead=%zu buffer=%s\n", valRead, buffer);
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
    cin >> awayMessage;

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
    cout << "\nAvailable commands: " << endl;
    cout << "1. Chat" << endl;
    cout << "2. Check Online Users (Check)" << endl;
    cout << "3. Set Away Message (Away)" << endl;
    cout << "4. Disconnect "<<endl;
    cout << "Any other number. Just to be online and be a good listener" << endl;
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
