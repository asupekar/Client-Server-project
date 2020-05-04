// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#define PORT 12150

using namespace std;

//int helloRPC(int & sock){
//    size_t valRead=0;
//    char hello[24];
//    strcpy(hello,"Hello from client");
//    char buffer[1024] = { 0 };
//    send(sock, hello, strlen(hello), 0);
//    printf("Hello message sent\n");
//    valRead = read(sock, buffer, 1024);
//    printf("ValRead=%d buffer=%s\n", valRead, buffer);
//    return 0;
//}
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

// rpc=disconnect;
void disconnectRPC(int & sock) {
    char const * disconnectCall = "rpc=disconnect;";
    cout << "Passing in: " << disconnectCall << endl;
    send(sock, disconnectCall, strlen(disconnectCall), 0);

    size_t valRead=0;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    printf("ValRead=%d buffer=%s\n", valRead, buffer);
    close(sock);

//    int result = close(sock);
//    cout << "Status = "<< result<<"; ";
//    if (result < 0) {
//        cout<<"Error = Closing failed!"<<endl;
//    } else {
//        cout<<"Error = Closing successful"<<endl;
//    }
//    return result;
}

int connectRPC(int & sock)
{
    // Input Arguments are:
    // username
    // password
    char username[50];
    char password[50];
    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    // input format="rpc=connect;username=<Your user>;password=<Your password>;"
//    int authStrLen = 32 + strlen(username) + strlen(password);
    char authStr[100];
    strcpy(authStr,"rpc=connect;username=");
    strcat(authStr, username);
    strcat(authStr, ";password=");
    strcat(authStr, password);
    strcat(authStr, ";");
    send(sock, authStr, strlen(authStr), 0);
    printf("Sent login details, waiting for server response...");
    cout << endl;

    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      ( This will be to blank or an error message)
    // output format="status=<error status>;error=<error or blank>
    size_t valRead=0;
    char buffer[1024] = { 0 };
    valRead = read(sock, buffer, 1024);
    printf("ValRead=%d buffer=%s\n", valRead, buffer);
    // parse here the response
    return 0;
}

int main(int argc, char const *argv[])
{
    int sock = 0;
    if(argc < 2) {
        printf("This needs 2 arguments Host name (127.0.0.1) and port #");
        return 1;
    }
    connectToServer((char *) argv[1], (char *) argv[2], sock);
    connectRPC(sock);
    cout<<"Waiting for 10 seconds....."<<endl;
    usleep(10000000);
    disconnectRPC(sock);
    return 0;

}