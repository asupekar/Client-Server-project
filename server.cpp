#include <cstdio>

// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>

using namespace std;

//typedef unordered_map<string, string> ul_map;
//ul_map readUsers();
//char *connectRPC(string string1, string string2, user data);
//void extractCredentials(string buffer, string &username, string &password);

#define PORT 12126

class parseAndStoreInput {
private:
    char * passedIn[4]{};
    unordered_map<string, string> parameters;
    string rpc;
public:
    explicit parseAndStoreInput(string rpcCall) {
        cout << "The RPC call in the constructor is " << rpcCall << endl;
        vector<string> vectorInput = initialInputParse(rpcCall);
        deepParse(vectorInput);
    };
    static vector<string> initialInputParse(string rpcCall) {
        vector<string> vec;
        string::size_type pos = string::npos;
        while((pos = rpcCall.find_first_of(";")) != string::npos){
            string str = rpcCall.substr(0, pos);
            vec.push_back(str);
            rpcCall.erase(0, pos+1);
        }
        return vec;
    }
    void deepParse(vector<string> vectorInput) {
        vector<string> vec;
        string::size_type pos = string::npos;
        for(string s : vectorInput) {
            while((pos = s.find_first_of("=")) != string::npos){
                string key = s.substr(0, pos);
                cout << key << endl;
                s.erase(0, pos+1);
                string val = s.substr(0, s.length());
                cout << val << endl;
                parameters.insert({key,val});
            }
        }
        unordered_map<string,string>::const_iterator got = parameters.find("rpc");
        rpc = got->second;
        parameters.erase("rpc");
    }
//    void initialInputParse(string rpcCall) {
////        cout << "The passed in RPC call is: " << rpcCall << endl;
//        int i = 0;
//        char *pch = strtok(rpcCall, ";");
//        while (pch != nullptr) {
//            passedIn[i++] = pch;
//            pch = strtok(nullptr, ";");
//        }
//    };
//    void deepParse(vector<string> vectorInput) {
//        string delimiter = "=";
//        for(int j = 0; j < 3; ++j) {
//            if(passedIn[j] != NULL) {
//                continue;
////                cout << passedIn[j] << endl;
//            } else {
//                break;
//            }
//        }
//        for(int j = 0; j < 3; ++j) {
//            if(passedIn[j] != NULL) {
//                string currCall(passedIn[j]);
//                string key = currCall.substr(0, currCall.find(delimiter));
//                string val = currCall.substr(currCall.find(delimiter) + 1, currCall.length());
//                parameters.insert({key,val});
//            } else {
//                break;
//            }
//        }
//        unordered_map<string,string>::const_iterator got = parameters.find("rpc");
//        rpc = got->second;
//        parameters.erase("rpc");
//    }

    string whichRPC() {
        return rpc;
    }

    unordered_map<string, string> restOfParameters() {
        return parameters;
    }

    void clear() {
        rpc = "";
        parameters.clear();
    }
};

// User class to store user information
class user {
private:
    // state?
    bool changed;
    string preferredUsername;
    string password;
    string userStatus;
    string userMessage;
public:
    user() {
        changed = false;
    }
    void initializeFields(string field, string val) {
        if(field == "password")
            password = val;
        else if(field == "userMessage")
            userMessage = val;
        else if(field == "preferredUserName")
            preferredUsername = val;
    }
    string getField(string field) {
        if(field == "password")
            return password;
        else if(field == "userMessage")
            return userMessage;
        else if(field == "preferredUserName")
            return preferredUsername;
        else if(field == userStatus)
            return userStatus;
        else
            return "Not valid field";
    }
//    void changeField(string field, string val) {
//        initializeFields(field, val);
//        changed = true;
//    }
};

// Read and store data into user object and then into map
class readAndStoreUserData {
private:
    unordered_map<string, user> userDict;
public:
    readAndStoreUserData() {
        readFile();
    };
    void readFile() {
        ifstream data("userInfo.csv");
        if (!data.is_open())
        {
            exit(EXIT_FAILURE);
        }
        string headerRef;
        getline(data,headerRef);

        string str;
        string header;

        while(getline(data, str)) {
            header = headerRef;
            unordered_map<string, string> userDetails;
            user currUser = user();

            string delimiter = ",";
            size_t pos = 0;
            size_t headerPos = 0;

            string token;
            string username;
            string currColumn;

            while ((pos = str.find(delimiter)) != string::npos) {
                headerPos = header.find(delimiter);
                token = str.substr(0, pos);
                currColumn = header.substr(0, headerPos);
                if (currColumn == "username") {
                    username = token;
                } else {
                    currUser.initializeFields(currColumn, token);
                }

                str.erase(0, pos + delimiter.length());
                header.erase(0, headerPos + delimiter.length());
            }
            if((header.at(header.length() - 1) = '\r')) {
                header.erase(header.length() - 1);
            }
            if(str.length() > 0 && (str.at(str.length() - 1) = '\r')) {
                str.erase(str.length() - 1);
            }
//            cout << "added a user" << endl;
            currUser.initializeFields(header, str);
//            cout << "added back to main map" << endl;
            userDict.insert({username,currUser});
        }
        data.close();
//        cout << "all data processed" << endl;
    };

    string getUserDetail(string un, string detail) {
//        cout << "start of method" << endl;
        string retrievedValue;
        user foundUser = findUser(un);
        retrievedValue = foundUser.getField(detail);
//        cout << "hello" << endl;
        return retrievedValue;
    };

//    void changeUserValue(string un, string detail, string value) {
//        user foundUser = findUser(un);
//        foundUser.changeField(detail, value);
//        userDict.erase(un);
//        userDict.insert({un,foundUser});
//    };

    user findUser(string un) {
        unordered_map<string,user>::const_iterator outerMapIt = userDict.find(un);
        user foundUser = outerMapIt->second;
        return foundUser;
    };

    string checkValidUsername(string un) {
        auto s = userDict.find(un);
        if(s == userDict.end()) {
            return "";
        } else {
            return un;
        }
    }

};

void connectRPC(readAndStoreUserData data, unordered_map<string, string> params, int &new_socket) {
    // prepare return
    char output[30];

    string storedUsername, storedPassword;
    string passedInUsername, passedInPassword;

    auto s = params.find("username");
    passedInUsername = s->second;
    storedUsername = data.checkValidUsername(passedInUsername);
    if (storedUsername != passedInUsername) {
        cout << "Bad Username passed in" << endl;
        strcpy(output, strcpy(output, "status=-1;error=BadUsername;"));
    } else {
        auto p = params.find("password");
        passedInPassword = p->second;
        storedPassword = data.getUserDetail(storedUsername, "password");
//        cout << storedPassword << endl;
        if (storedPassword == passedInPassword) {
            //success
            cout << passedInUsername << " has connected to the server!" << endl;
            strcpy(output, "status=1;error=Success;");
        } else {
            //error
            cout << "User found, but password was incorrect" << endl;
            strcpy(output, "status=-1;error=BadPassword;");
        }
    }
    send(new_socket, output, strlen(output), 0);
};

void disconnectRPC(int &new_socket) {
    char const *disconnect = "Disconnected";
    send(new_socket, disconnect, strlen(disconnect), 0);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

    cout << "Server startup" << endl;

    readAndStoreUserData data = readAndStoreUserData();

    cout << "All data read in" << endl;
//    cout << data.getUserDetail("Ruifeng", "password") << endl;
//    ul_map userList = readUsers();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Got Socket\n");
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    printf("About to bind\n");
    if (::bind(server_fd, (struct sockaddr *)&address,
               sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while(true) {
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        printf("Waiting\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                                 (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        while ((valread = read(new_socket, buffer, 1024)) != 0) {
            cout << "Waiting for new RPC" << endl;
//            cout << "The buffer is " << buffer << endl;
//            cout << "The buffer size is " << strlen(buffer) << endl;

            parseAndStoreInput input = parseAndStoreInput(buffer);
//            cout << "The RPC is: " << input.whichRPC() << endl;
            if(input.whichRPC() == "connect") {
//                cout << "entered connect correctly" << endl;
                unordered_map<string, string> maps = input.restOfParameters();
                connectRPC(data, maps, new_socket);
                input.clear();
//                cout << "The rpc is now: " << input.whichRPC() << endl;
            } else if(input.whichRPC() == "disconnect") {
                disconnectRPC(new_socket);
                input.clear();
                cout << "Disconnect called" << endl;
            } else {
                char const *unknownRPC = "Unrecognized RPC detected";
                send(new_socket, unknownRPC, strlen(unknownRPC), 0);
                input.clear();
            }
        }
    }
};