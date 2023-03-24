#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define PORT 9001 // THIS IS THE SERVER PORT NUMBER                  
#define SERVER_IP "127.0.0.1" // THIS IS THE SERVER IP ADDRESS       


/*
This is the client_exec source file for the file transfer application.

This client will connect to a server, prompt the user to enter a the filename they want from the server, then send the filename to the server. 

The server checks if the file exists, if so, it opens the file and sends the contents line-by-line, which the client then writes each line to out.txt

After each line is sent, the server sends EOT, then terminates.
*/


void ask_for_file(char* filename) {
	/*
	Parameters: 1. filename	-> requested file name
	Descirption: Asks the user for the file they want
	*/
	puts("Hi, what file would you like to request from the server?\n");
	fgets(filename,39,stdin);
}

int main(void) {
	int total_data_bytes = 0; 		// Count for each data byte recieved from server
	char filename[99]; 				// String to store the requested filename
	int filename_len; 				// Used to set the end of filename string to null
	unsigned short header[] = {0,0};	// Header packet {count,sequence} 
	char data[80];  				// Data buffer that will conatin the line of file recieved
	FILE *out; 						// File that recieved packets will be to be written to

	int queueLimit = 99;
	struct sockaddr_in addrport;
	int addrlen = sizeof(addrport);

	int sockid = socket(PF_INET,SOCK_STREAM,0);
	if (sockid == -1){
		puts("Error creating socket");
		exit(0);
	}

	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT);
	addrport.sin_addr.s_addr = inet_addr(SERVER_IP);

	int s = connect(sockid,&addrport,addrlen);
	if (s == -1){
		puts("Error connecting to server");exit(0);
	}

	ask_for_file(filename); // Ask user for filename they want from the server
	filename_len = strlen(filename); // Get filename length
	filename[filename_len] = 0; // Null the byte after the filename in the string buffer

	out = fopen("out.txt" , "a+"); // Create and/or open the out.txt file in the server file directory

	// Send initial file request to server
	int count = send(sockid,filename,filename_len-1,0);
	if (count == -1) {
		puts("Error sending request");
		close(sockid);
		exit(0);
	}

	while (count != 0) {
		// Recieve header packet 
		count = recv(sockid,header,4,0);
		if (count == -1) {
			puts("Error recieving header");
			break;
		} else {
			if (header[0] != 0) printf("Packet %i recieved with %i  data bytes\n",header[1],header[0]);
		}
		
		// If first byte of header is 0, EOT 
		if (header[0] == 0) {
			printf("End of Transmission Packet with sequence number %i recieved with %i data bytes\n",header[1],header[0]);
			break; 
		}
		
		// Recieve data packet
		count = recv(sockid,data,header[0],0);		
		if (count == -1) {
			puts("Error recieving header");
			break;
		}
		
		// Set byte after line text to null, only write up to this byte into the out.txt file
		data[header[0]] = 0; 
		fputs(data,out);
		total_data_bytes += strlen(data);
		
		// send Acknowledgement to server
		count = send(sockid,"ack",3,0);
		if (count == -1) {
			puts("Error sending request");
			break;
		}
	}

	// Close the file and socket
	fclose(out);
	close(sockid);

	printf("Number of data packets recieved\n%i\n",total_data_bytes);

	return 0;
}
