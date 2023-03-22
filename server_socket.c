#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001

int set_up_connection(int sockid, struct sockaddr_in addrport,int addrlen) {
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT);
	addrport.sin_addr.s_addr = htonl(INADDR_ANY);

	int status = bind(sockid, &addrport, addrlen);
	if (status == -1) {
		puts("Error binding");
		close(sockid);
		exit(0);
	}
}

int get_total_lines(char *client_message,short num_of_lines) {
	FILE *request = fopen(client_message, "r");
	char file_character;

	for (file_character = getc(request); file_character != EOF; file_character = getc(request)) {
		if (file_character == '\n') num_of_lines++;
	}
	return num_of_lines;
}

void get_file_line(char *client_message, char *data, short line) {
		FILE *request = fopen(client_message, "r");

		for(int i = 0; i<line+1; i++) {
   			fgets(data, 80, (FILE*)request);
		}
}

void EOT(int sockid,int s, char *header, int total_bytes_sent) {
	send(s,"",0,0);
	printf("End of Transmission Packet with sequence number %i transmitted with %i data bytes\n", header[1],header[0]);
	close(sockid);
	printf("Number of data packers transmitted\n%i\n",total_bytes_sent);
}

int main(void) {
	int total_bytes_sent = 0;
	char header[2];
	header[0] = 0x0;
	header[1] = 0x0;
	char data[80];
	char client_message[40];
	char ack[3];
	int queueLimit = 99;
	short num_of_lines = 0;
	struct sockaddr_in addrport;
	int addrlen = sizeof(addrport);

	int sockid = socket(PF_INET,SOCK_STREAM,0);
	if (sockid == -1) {puts("Error creating socket");exit(0);}

	int status = set_up_connection(sockid,addrport,addrlen);

	status = listen(sockid,queueLimit);
	if (status == -1){
		puts("Error listening");
		close(sockid);
		exit(0);
	}


	int s = accept(sockid,&addrport,&addrlen);
	if (s == -1) {
		puts("Error accepting");
		close(sockid);
		exit(0);
	}

	int count = recv(s,client_message,40,0);
	if (count == -1) {
		puts("Error recieving");
	} else {
		if (header[1]==0) num_of_lines = get_total_lines(client_message,num_of_lines);
	}

	while (count != 0) {

		get_file_line(client_message,data,header[1]); 

		header[1] += 1;
		if (header[1]>num_of_lines) {
			header[0] = 0;
			*data = "";
		} else {
			header[0] = strlen(data);
			total_bytes_sent += header[0];
		}

		count = send(s,header,2,0);
		if (count == -1) {
			puts("Error sending header");
			break;
		}

		count = send(s,data,header[0],0);
		if (count == -1) {
			puts("Error sending data");
			break;
		} else {
			printf("Packet %i transmitted with %i data bytes\n", header[1],header[0]);
		}
		
		int count = recv(s,ack,3,0);
		if (count == -1) {
			puts("Error recieving");
			break;
		}
	}

	EOT(sockid,s,header,total_bytes_sent); 

	return 0;
}
