
CAUTION: Don't run this with SUDO, client can steal protected files :)

Relevant files and directories:

    /CharlesAdams/ (root)
        |
        |-> flag.txt :)
        |    
        |-> Makefile (to ease compilation)
        |         
        |-> /client/
        |       |
        |       |->client_exec (client executable)
        |       |
        |       |->client_socket.c (client source file)
        |       |
        |       |->out.txt (file for data to be transferred into)
        |
        |-> /server/
                |
                |->server_exec (server executable)
                |
                |->server_socket.c (server source file)
                |
                |->transfer.txt (file for data to be transferred out of)

CHANGING SERVER PORT AND IP ADDRESS

    Server port and ip addresses are hard coded into the client/server source files as follows:
        #define PORT 9001 
        #define ADDRESS "127.0.0.1"


TO COMPILE:

    Compiling using make
        1. Start a linux terminal
        2. Navigate to the program's root directory, /CharlesAdams/
        3. Enter the command `make build`

        OR
	
    Compiling manually
        1. Navigate to the program's server directory, /CharlesAdams/server
        2. enter the command `gcc -o ./server/server_exec ./server/server_socket.c`  
        3. Navigate to the program's client directory, /CharlesAdams/client  
        4. Enter the command `cc -o ./client/client_exec ./client/client_socket.c `    

TO RUN: 

    To successfully run the program, we must first start the server, then start the client, in two seperate terminals.

        1. Start server (after compilation)
            a. Start a Linux terminal (or use the terminal from compilation steps)
            b. Navigate to the program's server directory, /CharlesAdams/server/
            c. Enter the command `./server_exec`

        2. Start client (after compilation)
            a. Start a new Linux terminal
            b. Navigate to the program's client directory, /CharlesAdams/client/
            c. Enter the command `./client_exec`

        3. Request a file from the server (when server and client are in execution)
            Note: When the client prompts the user to enter a file to request from the server, a connection has been successfully established
            a. In the terminal running the client service, type the file name you wish to retreive from the server (transfer.txt is ready and waiting on the server to be transferred)
            b. Press Enter
            c. File contents will write to out.txt in the client's directory
