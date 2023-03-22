#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001
#define SERVER_IP "127.0.0.1"

char* ask_for_file(char* filename) {
	puts("Hi, what file would you like to request from the server?\n");
	fgets(filename,39,stdin);
}

int set_up_connection(int sockid, struct sockaddr_in addrport,int addrlen) {
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT);
	addrport.sin_addr.s_addr = inet_addr(SERVER_IP);
	int s = connect(sockid,&addrport,addrlen);
	if (s == -1) {puts("Error connecting to server");exit(0);}
	return s;
}

int main(void) {
	int total_data_bytes = 0;
	char filename[40];
	int filename_len;
	char header[2];
	char data[80];
	FILE *out;
	int queueLimit = 99;
	struct sockaddr_in addrport;
	int addrlen = sizeof(addrport);
	short last_sequence_num = -1;


	int sockid = socket(PF_INET,SOCK_STREAM,0);
	if (sockid == -1) {
		puts("Error creating socket");
		exit(0);
	}

	int s = set_up_connection(sockid,addrport,addrlen);

	ask_for_file(filename);
	filename_len = strlen(filename);
	filename[filename_len] = 0;
	out = fopen("out.txt" , "a+");
	int count = send(sockid,filename,filename_len-1,0);
	if (count == -1) {
		puts("Error sending request");
		close(sockid);
		exit(0);
	}

	while (count != 0) {
		// send filename first send, ack packets thereafter

		count = recv(sockid,header,2,0);
		if (count == -1) {
			puts("Error recieving header");
			break;
		} else {
			printf("Packet %i recieved with %i data bytes\n",header[1],header[0]);
		}
		
		if (header[0] == 0) {
			printf("End of Transmission Packet with sequence number %i recieved with %i data bytes\n",header[1],header[0]);
			break; 
		}
		
		count = recv(sockid,data,header[0],0);		
		if (count == -1) {
			puts("Error recieving header");
			break;
		}
		
		data[header[0]] = 0; 
		total_data_bytes += strlen(data);
		fputs(data,out);
		
		count = send(sockid,"ack",3,0);
		if (count == -1) {
			puts("Error sending request");
			break;
		}
	}

	fclose(out);
	close(sockid);

	printf("Number of data packets recieved\n%i\n",total_data_bytes);

	return 0;
}
