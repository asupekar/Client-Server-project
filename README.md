# Client-Server-project

I have created my disconnectRPC and the entire program is running as per required.
I also have edited connectRPC on server side because it did not validate for case sensitive username and password. I was not known about the fact that namoi was also working on this.
I have parsed the command line args when the client enters its username and password and checked if its valid by comapring it with the values in the unordered map "users" used in the server program.
I have added the logic for connectRPC, wait for 10 seconds and then disconnect the client from the server. After disconnecting the client from the server, the server waits indefinitely for other client connection.
I had some questions which will ask on Wednesday like....
1:- Do we have to hard code username and passswords before the client runs?
