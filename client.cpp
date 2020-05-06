// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

#define PORT 12126

using namespace std;

// If user does not pass in information, still allow log in (FOR NOW CAN BE REMOVED LATER)
char const * host = "127.0.0.1";
char const * port = "12126";

// Making the initial connection to the server
int connectToServer(char *szHostName, char *szPort, int & sock)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    uint16_t port = (uint16_t) atoi(szPort);
    serv_addr.sin_port = htons(port);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");return -1;
    }
    return 0;
}

// Client side disconnect RPC
// Sends "rpc=disconnect;" to the server for processing and receives a buffer back with "Disconnected"
// Then processes the disconnect
void disconnectRPC(int & sock) {
    char disconnectCall[256];
    strcpy(disconnectCall,"rpc=disconnect;");
    cout << "Passing in: " << disconnectCall << endl;
    cout << "String length being passed in: " << strlen(disconnectCall) << endl;
    send(sock, disconnectCall, strlen(disconnectCall)+1, 0);

    size_t valRead=0;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    printf("ValRead=%d buffer=%s\n", valRead, buffer);
    close(sock);
}

// Parses buffer and returns the status or error message as a string
vector<string> getStatusorError(string buffer) {
    cout << "Buffer being passed in is " << buffer << endl;
    vector<string> vec;
    string::size_type pos = string::npos;
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
string connectRPC(int & sock)
{
    // Input Arguments are: username, password
    char username[256];
    char password[256];
    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    // Formulates input to send to server
    // input format="rpc=connect;username=<Your user>;password=<Your password>;"
    char authStr[256];
    strcpy(authStr,"rpc=connect;username=");
    strcat(authStr, username);
    strcat(authStr, ";password=");
    strcat(authStr, password);
    strcat(authStr, ";");
    send(sock, authStr, strlen(authStr)+1, 0);
    cout << "Sent login details, waiting for server response..." << endl;

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      ( This will be to blank or an error message)
    // output format="status=<error status>;error=<error or blank>
    size_t valRead=0;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    printf("ValRead=%d buffer=%s\n", valRead, buffer);

    //TODO: Change to parse the buffer and return the correct status
    return buffer;
}

int main(int argc, char const *argv[])
{
    int sock = 0;
    if(argc < 2) {
        cout << "This needs 2 arguments Host name (127.0.0.1) and port #" << endl;
        connectToServer((char *)host, (char *) port, sock);
    } else {
        connectToServer((char *) argv[1], (char *) argv[2], sock);
    }
    bool connected = false;
    do {
        string response = connectRPC(sock);
        vector<string> parsedResponse = getStatusorError(response);
        if (parsedResponse[1] == "1") {
            cout << "Successfully connected!" << endl;
            connected = true;
        } else {
            cout << parsedResponse[3] << endl;
//            cout << "Fail" << endl;
        }
    } while(!connected);
    cout<<"Waiting for 10 seconds....."<<endl;
    usleep(10000000);
    disconnectRPC(sock);
    return 0;

}