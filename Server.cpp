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
using namespace std;

typedef unordered_map<string, string> ul_map;
ul_map readUsers();
char *connectRPC(string string1, string string2, ul_map map);
void extractCredentials(string buffer, string &username, string &password);

#define PORT 12124
int main(int argc, char const *argv[]) {
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = { 0 };

  cout << "Server startup" << endl;

  ul_map userList = readUsers();

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
      printf("Accepted Connected\n");

      // We will read the very simple HELLO message and return a Hello message back

      while ((valread = read(new_socket, buffer, 1024)) != 0) {
          printf("%s\n", buffer);

          string username, password;
          extractCredentials((string)buffer, username, password);

          char *output = connectRPC(username, password, userList);

          send(new_socket, output, strlen(output), 0);
      }
  }
}

void extractCredentials(string buffer, string &username, string &password) {
    string delimiter = ";";
    size_t pos = 0;
    string *tokens = new string[3];
    int i = 0;
    while ((pos = buffer.find(delimiter)) != std::string::npos && i < 3) {
        tokens[i] = buffer.substr(i, pos);
        cout<<"tokens = "<<i<< " "<<tokens[i]<<endl;
        buffer.erase(2, pos + delimiter.length());
        i++;
    }
    username = tokens[1].substr(tokens[1].find("=")+1,tokens[1].find(";")-tokens[1].find("=")-1);
    cout<<"Username = "<<username<<endl;
    password = tokens[2].substr(tokens[2].find("=")+1,tokens[2].find(";")-tokens[2].find("=")-1);
    cout<<"Password = "<<password<<endl;
}

char* connectRPC(string userName, string password, ul_map ul) {
    // prepare return
    char* output = new char[30];
    string lookupUn = userName;
    strcpy(output, "status=1;error=Success;");
    try {
        ul.at(lookupUn);
        // validate passwords
        cout<<"";
        string pwd = ul[lookupUn];
        if (pwd.compare(password) == 0) {
            //success
            strcpy(output, "status=1;error=Success;");
        } else {
            //error
            strcpy(output, "status=-1;error=BadPassword;");
        }
    } catch (...) {
        //error
        strcpy(output, "status=-1;error=BadUsername;");
    }
    return output;
}
// including a function to read/write user list to disk
// simple but users will persist between server launches
// load at server start
    ul_map readUsers() {
        //commented out csv portion because it was reading the \n
        /*//initialize
        ul_map users;
        //open the csv file with users info
        ifstream openFile("users.csv");
        //loop over and read to a hashmap
        if(openFile.is_open()) {
            while(true) {
                string key, value;
                if (!getline(openFile, key, ',')) break;
                //read value
                getline(openFile, value, '\n');
                //cout << key << ", " << value << endl;
                users[key] = value;
            }
            openFile.close();
        }*/
        ul_map users {{{"Naomi","p@ssword"},{"Ruifeng","p@ssword"},{"Aishwarya","p@ssword"}}};
        return users;
    }