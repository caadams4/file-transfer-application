#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001 // THIS IS THE SERVER PORT NUMBER                  


/*
This is the server_exec source file for the file transfer application.

This server will wait for a client to connect. Once a client service connects, they will send the file name that they wish to retrieve from this server. 

The server checks if the file exists, if so, it opens the file and sends the contents line-by-line. After each line is sent, the server sends EOT, then terminates.
*/



int get_total_lines(char *client_message, int num_of_lines, FILE *requested_file)
{
	/*
	Parameters: 1. client_message	-> requested file name
				2. num_of_lines 	-> number of lines in the requested file

	Descirption: Function checks if file exists, if not it returns and preps for EOT. If the file is
	found, it opens the file and counts the endline (\n) characters to determine how many lines to send.

	Return: number of lines in file
	*/

	char file_character; // Var to check if character in file is endline char

	for (file_character = getc(requested_file); file_character != EOF; file_character = getc(requested_file))
	{
		// check each char to determine how many lines are in a file.
		if (file_character == '\n')
			num_of_lines++;
	}
	return num_of_lines;

}

void get_file_line(char *client_message, char *data, int line)
{
	/*
	Parameters: 1. client_message	-> requested file name
				2. data 			-> data buffer to become the data packet
				3. line				-> which line to send
	Descirption: Function opens a file, goes to the line to send, stores it in data
	*/
	FILE *requested_file = fopen(client_message, "r");

	for (int i = 0; i < line + 1; i++)
	{   // get lines until the line required is reached
		fgets(data, 80, (FILE *)requested_file);
	}

}

void EOT(int sockid, char *header, int total_bytes_sent)
{
	/*
	Parameters: 1. socketid			-> server socket
				2. header			-> for logging how many packets sent EOT
				3. total_bytes_sent -> for logging total data packets sent
	Descirption: Logs EOT and closes server socket
	*/

	printf("End of Transmission Packet with sequence number %i transmitted with %i data bytes\n", header[1], header[0]);
	close(sockid);
	printf("Number of data packers transmitted\n%i\n", total_bytes_sent);
}

int main(void)
{
	int total_bytes_sent = 0;		// Count for each data byte sent to client
	unsigned short header[] = {0,0};	// Header packet {count,sequence} 
	char data[80];			 		// Data buffer that will conatin the line of file to be sent
	char client_message[40]; 		// String to contian the file name the client is requesting
	char ack[3];			 		// Acknowledgement that the client recieved the last message
	short num_of_lines = 0;  		// Contains how many lines to send
	FILE *requested_file;	 		// file to be opened, read, and sent
	int queueLimit = 99;
	struct sockaddr_in addrport;
	int addrlen = sizeof(addrport);


	int sockid = socket(PF_INET, SOCK_STREAM, 0);
	if (sockid == -1){
		puts("Error creating socket");
		exit(0);
	}

	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT);
	addrport.sin_addr.s_addr = htonl(INADDR_ANY);

	int status = bind(sockid, &addrport, addrlen);
	if (status == -1){
		puts("Error binding");
		close(sockid);
		exit(0);
	}

	status = listen(sockid, queueLimit);
	if (status == -1){
		puts("Error listening");
		close(sockid);
		exit(0);
	}

	int s = accept(sockid, &addrport, &addrlen);
	if (s == -1){
		puts("Error accepting");
		close(sockid);
		exit(0);
	}

	// Recieve initial client request for file
	int count = recv(s, client_message, 40, 0);
	if (count == -1){
		puts("Error recieving request for file");
	}else{
		if (access(client_message, F_OK) == -1){  // check if file exists
			puts("File not found");
			// Exit if file not found, prep for EOT
			count = 0;
		}else{
			// open requested file
			requested_file = fopen(client_message, "r");
			// Count the number of lines in the file, if file does not exist, return -1
			num_of_lines = get_total_lines(client_message, num_of_lines, requested_file);
		}
	}

	while (count != 0){
		// get a line from the file, store it in data buffer
		get_file_line(client_message, data, header[1]);
		// increment sequence number
		header[1] += 1;
		if (header[1] > num_of_lines+1){ // if all lines sent
			// close file, send complete
			fclose(requested_file);
			// if sequence number is greater that number of lines, configure packets for EOT
			header[0] = 0;
			*data = "";
		}
		else{
			// else, get ready to send line
			header[0] = strlen(data);
			total_bytes_sent += header[0];
		}

		// Send header
		count = send(s, header, 4, 0);
		if (count == -1){
			puts("Error sending header");
			break;
		}

		// Send data
		count = send(s, data, header[0], 0);
		if (count == -1){
			puts("Error sending data");
			break;
		}else{
			if (header[0] != 0) printf("Packet %i transmitted with %i data bytes\n", header[1], header[0]);
		}

		// Recvieve client ack
		int count = recv(s, ack, 3, 0);
		if (count == -1){
			puts("Error recieving");
			break;
		}
	}

	EOT(sockid, header, total_bytes_sent);

	return 0;
}
