// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>

using namespace std;

#define PORT 12126

// Class to encrypt and decrypt the password
class Encryption {

private:
    const static int shift = 4;
    const static int OFFSET_MIN = 32;
    const static int OFFSET_MAX = 126;

public:
    // Checks if all characters in the text are valid
    // Encrypts the passed in text based on the predefined shift
    static string encrypt(string plainText) {
        for(char const &c: plainText){
            int index = (int)c;
            if (!isPositionInRange(index)) {
                throw invalid_argument("String to be encrypted has unrecognized character ");
            }
        }
        string result = encryptDecrypt(plainText, true);
        return result;
    }

    // Decrypts the passed in text
    static string decrypt(string cipherText) {
        string result = encryptDecrypt(cipherText, false);
        return result;
    }

    // Checks if the position of the offset is in range
    static bool isPositionInRange(int index) {
        if(index < OFFSET_MIN || index > OFFSET_MAX){
            return false;
        }
        return true;
    }

    // Encrypts and decrypts based on the flag (true for encrypt, false for decrypt)
    // Returns the encrypted or decrypted word
    static string encryptDecrypt(string word, bool flag) {
        string result;
        int deShift = 0;
        for(long unsigned int i =0; i < word.length();i++) {
            if (isupper(word[i])) {
                if(flag == true) {
                    result = result + char(int(word[i] + shift - 65) % 26 + 65);
                }
                else{
                    deShift = 26 - shift;
                    result = result + char(int(word[i] + deShift - 65) % 26 + 65);
                }
            } else if (islower(word[i])){
                if(flag == true) {
                    result = result + char(int(word[i] + shift - 97) % 26 + 97);
                }
                else {
                    deShift = 26 - shift;
                    result = result + char(int(word[i] + deShift - 97) % 26 + 97);
                }
            } else if(isdigit(word[i])){
                if(flag == true) {
                    result = result + char(int(word[i] + shift - 48) % 10 + 48);
                }
                else {
                    deShift = 10 - shift;
                    result = result + char(int(word[i] + deShift - 48) % 10 + 48);
                }
            }
            else {
                if(flag == true) {
                    result = result + char(int(word[i] + shift - 32) % 15 + 32);
                }
                else {
                    deShift = 15 - shift;
                    result = result + char(int(word[i] + deShift - 32) % 15 + 32);
                }
            }
        }
        return result;
    }
};

// Class to parse and store the input string
class parseAndStoreInput {
private:
    // Map to store parameters by format <parameterName, parameterValue>
    unordered_map<string, string> parameters;
    // Which RPC the parameters are associated with for easy access
    string rpc;
public:
    // Constructor
    explicit parseAndStoreInput(string rpcCall) {
//        cout << "The RPC call in the constructor is " << rpcCall << endl;
        vector<string> vectorInput = initialInputParse(move(rpcCall));
        deepParse(vectorInput);
    };
    // Initial parse of the RPC call. Parses by ; to separate out all of the arguments
    // End result will be a vector of format: [rpc, rpcType, parameter1, parameter1value, etc...]
    static vector<string> initialInputParse(string rpcCall) {
        vector<string> vec;
        string::size_type pos;
        while((pos = rpcCall.find_first_of(';')) != string::npos){
            string str = rpcCall.substr(0, pos);
            vec.push_back(str);
            rpcCall.erase(0, pos+1);
        }
        return vec;
    }
    // Secondary parse of the parameters inserting them into the parameters map
    // Map will have key, value pairs of {parameter, parameterValue}
    // The key, value pair of {rpc, rpcType} will be removed from the map as rpc value will be stored in the obj field
    void deepParse(const vector<string>& vectorInput) {
        string::size_type pos;
        for(string s : vectorInput) {
            while((pos = s.find_first_of('=')) != string::npos){
                string key = s.substr(0, pos);
//                cout << key << endl;
                s.erase(0, pos+1);
                string val = s.substr(0, s.length());
//                cout << val << endl;
                parameters.insert({key,val});
            }
        }
        unordered_map<string,string>::const_iterator got = parameters.find("rpc");
        rpc = got->second;
        parameters.erase("rpc");
    }
    // Returns which rpc is associated with the object
    string whichRPC() {
        return rpc;
    }
    // Returns the unordered map of parameters
    unordered_map<string, string> restOfParameters() {
        return parameters;
    }
    // Clears the object's rpc type and parameter map
    void clear() {
        rpc = "";
        parameters.clear();
    }
};

// User class to store user information
class user {
private:
    // state?
//    bool changed;
    string preferredUsername;
    string password;
    string userStatus;
    string userMessage;
public:
    // Default constructor
    user() = default;
    // Initializes the fields that are passed in
    void initializeFields(const string& field, const string& val) {
        if(field == "password")
            password = val;
        else if(field == "userMessage")
            userMessage = val;
        else if(field == "preferredUserName")
            preferredUsername = val;
    }
    // Returns the value stored for this user under requested field
    string getField(const string& field) {
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

//    TODO: Milestone2    
//    void changeField(string field, string val) {
//        initializeFields(field, val);
//        changed = true;
//    }
};

// Read and store data into user object and then into map
class readAndStoreUserData {
private:
    // Unordered map to store all users by <username, userObject>
    unordered_map<string, user> userDict;
public:
    //Constructor
    readAndStoreUserData() {
        readFile();
    };
    // Read from CSV, parse and store in user object
    void readFile() {
        // The file of user info to be loaded from
        ifstream data("userInfo.csv");
        // Checks if file was opened successfully, if not, the program will fail
        if (!data.is_open())
        {
            exit(EXIT_FAILURE);
        }
        // Accesses and saves the header row for use
        string headerRef;
        getline(data,headerRef);

        // Temp storage for next row of data being accessed
        string str;
        string header;

        // Continues running while CSV file still has more rows
        while(getline(data, str)) {
            header = headerRef;
            unordered_map<string, string> userDetails;
            user currUser = user();

            string delimiter = ",";
            size_t pos;
            size_t headerPos;

            string token;
            string username;
            string currColumn;
            // Parses current row
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
            // End of header row cleanup
            if((header.at(header.length() - 1) = '\r')) {
                header.erase(header.length() - 1);
            }
            // End of current row cleanup
            if(str.length() > 0 && (str.at(str.length() - 1) = '\r')) {
                str.erase(str.length() - 1);
            }
//            cout << "added a user" << endl;
            // Pushes values to user object
            currUser.initializeFields(header, str);
//            cout << "added back to main map" << endl;
            // Adds keyVal pair to map
            userDict.insert({username,currUser});
        }
        data.close();
//        cout << "all data processed" << endl;
    };

    // Gets the desired user detail for a specific username
    string getUserDetail(const string& un, const string& detail) {
        string retrievedValue;
        user foundUser = findUser(un);
        retrievedValue = foundUser.getField(detail);
        return retrievedValue;
    };

//    TODO: Milestone2
//    void changeUserValue(string un, string detail, string value) {
//        user foundUser = findUser(un);
//        foundUser.changeField(detail, value);
//        userDict.erase(un);
//        userDict.insert({un,foundUser});
//    };

    // Finds the user object from the map based on provided username
    user findUser(const string& un) {
        unordered_map<string,user>::const_iterator outerMapIt = userDict.find(un);
        user foundUser = outerMapIt->second;
        return foundUser;
    };

    // Checks if the username provided exists in the map
    string checkValidUsername(string un) {
        auto s = userDict.find(un);
        if(s == userDict.end()) {
            return "";
        } else {
            return un;
        }
    }
};

// Server side connect RPC
void connectRPC(readAndStoreUserData data, unordered_map<string, string> params, int &new_socket) {
    // prepare return
    char output[30];
    // Creation of encryption object
    Encryption enc;
    string storedUsername, storedPassword;
    string passedInUsername, passedInPassword;
    // Looks in map of parameters to get the passed in username
    auto s = params.find("username");
    passedInUsername = s->second;
    // Grabs the stored username. Will match if the passed in username is valid, if not will be invalid
    storedUsername = data.checkValidUsername(passedInUsername);
    // Case: Passed in username does not exist in system
    if (storedUsername != passedInUsername) {
        cout << "Bad Username passed in" << endl;
        strcpy(output, strcpy(output, "status=-1;error=BadUsername;"));
    // Case: Passed in username exists
    } else {
        // Checking password
        // Looks in map of parameters to get the passed in password
        auto p = params.find("password");
        passedInPassword = p->second;
        // Gets the stored password and decrypts it
        storedPassword = enc.decrypt(data.getUserDetail(storedUsername, "password"));
//        cout << storedPassword << endl;
        // Case: Stored password matches the passed in password
        if (storedPassword == passedInPassword) {
            //success
            cout << passedInUsername << " has connected to the server!" << endl;
            strcpy(output, "status=1;error=Success;");
        // Case: Stored password does not match the passed in password
        } else {
            //error
            cout << "User found, but password was incorrect" << endl;
            strcpy(output, "status=-1;error=BadPassword;");
        }
    }
    // Sends result back to client
    send(new_socket, output, strlen(output)+1, 0);
};

// Server side disconnect RPC
void disconnectRPC(int &new_socket) {
    char const *disconnect = "status=0;error=disconnected";
    send(new_socket, disconnect, strlen(disconnect)+1, 0);
}

// Server side sendmessage RPC
void sendMessageRPC(readAndStoreUserData data, unordered_map<string, string> params, int &new_socket) {
    // prepare return
    char output[30];
    // Creation of encryption object
    Encryption enc;
    string storedUsername, storedPassword;
    string passedInUsername, message;
    // Looks in map of parameters to get the passed in username
    auto s = params.find("toUser");
    passedInUsername = s->second;
    storedUsername = data.checkValidUsername(passedInUsername);
    // Grabs the stored username. Will match if the passed in username is valid, if not will be invalid
    // Case: Passed in username does not exist in system
    if (storedUsername != passedInUsername) {
        cout << "Bad Username passed in" << endl;
        strcpy(output, strcpy(output, "status=-1;error=BadUsername;"));
    // Case: Passed in username exists
    } else {
        // Checking password
        // Looks in map of parameters to get the passed in password
        auto p = params.find("message");
        message = p->second;
        cout << "Message from client: " << message << endl;
        strcpy(output, "status=1;error=Success;");
    }

    // sendMesssageTo(passedInUsername, message, new_socket);
    
    //testing a wait
    usleep(5000000);
    
    // Sends result back to client
    send(new_socket, output, strlen(output)+1, 0);
};

// ServerContextData in example
// Information that is shared to all threads
class SharedServerData {
private:
    pthread_mutex_t lock;
    pthread_cond_t fill;
    int socket;
    
public:
    //public for now
    readAndStoreUserData userDataStore;

    SharedServerData() 
    {
        socket = 0;
    }

    // necessary socket info for threading
    void setSocket(int s)
	{
		this->socket = s;
	}
    int getSocket()
	{
		return socket;
	}
};

// Server class to store server functionality
class Server {
private:
	int server_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int port;
    int maxConn;
    readAndStoreUserData userDataStore;
    SharedServerData *sharedData;

public:

    // Constructors
	Server(int nPort)
	{
		port = nPort;
        //arbitrary value for max connections
        maxConn = 5;
	}

	~Server()
	{

	}

    // Function: start server functionality
	int startServer()
	{
		userDataStore = readAndStoreUserData();
        bindSocket(port);
        sharedData = new SharedServerData();
		return 0;
	}

    // Function: Bind server to port and start listening
    int bindSocket(int port) {
        int opt = 1;

		// Creating socket file descriptor 
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		{
			perror("socket failed");
			exit(EXIT_FAILURE);
		}

		// Forcefully attaching socket to the port 8080 
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt)))
		{
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = (uint16_t) htons((uint16_t) port);

		if (bind(server_fd, (struct sockaddr *)&address,
			sizeof(address)) < 0)
		{
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		if (listen(server_fd, maxConn) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
        return 0;
    }

	// Function: new client connection
	int acceptNewConnection()
	{
		int new_socket;

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
			(socklen_t*)&addrlen)) < 0)
		{
			perror("accept");
			return (-1);
		}
		return new_socket;
	}

    // Function: copied, expecting additional functionality
	int closeServer()
	{
		return 0;
	}

    // Function: listens and controls threads
    int threadLoop() {
        // listen for connections
        int i = 0;
        // create thread array
        pthread_t threads[maxConn];
        
        while (true) {
            // accept a new request
            int newConn = acceptNewConnection();
            sharedData->setSocket(newConn);
            pthread_create(&threads[i], NULL, rpcThread, (void *) sharedData);
            i = (i+1) % maxConn;
        }
    }

    // Function to manage a thread
   static void *rpcThread(void *arg)
    {
        int valread;
        int sock;
        char buffer[1024] = { 0 };

        // read data
        SharedServerData *pSharedData = (SharedServerData *) arg;
        sock = pSharedData->getSocket();

        // User continuously listens for new RPC requests from connected user
        while ((valread = read(sock, buffer, 1024)) != 0) {

            // The buffer is parsed and stored in an object
            parseAndStoreInput input = parseAndStoreInput(buffer);

            // Checking on which RPC has been called
            // Connect called
            if(input.whichRPC() == "connect") {
                cout << "Connect called" << endl;
                unordered_map<string, string> maps = input.restOfParameters();
                connectRPC(pSharedData->userDataStore, maps, sock);
                input.clear();
            // Disconnect called
            } else if(input.whichRPC() == "disconnect") {
                cout << "Disconnect called" << endl;
                disconnectRPC(sock);
                input.clear();
                pthread_exit(NULL);
            // Send message called
            }
            else if(input.whichRPC() == "sendmessage") {
                cout << "Send message called" << endl;
                unordered_map<string, string> maps = input.restOfParameters();
                sendMessageRPC(pSharedData->userDataStore, maps, sock);
                input.clear();
            // Unknown RPC called
            } 
            else {
                char const *unknownRPC = "status=-1;error=unknownRPC";
                send(sock, unknownRPC, strlen(unknownRPC)+1, 0);
                input.clear();
            }
        }
        //pthread_exit(NULL);
        return NULL;
    }


};

int main(int argc, char const *argv[]) {
    // build a Server object
    int nPort = atoi((char const  *)argv[1]);
    Server *server = new Server(nPort);

    // reads in user data
    // start listening
    server->startServer();
    server->threadLoop();
    
}
