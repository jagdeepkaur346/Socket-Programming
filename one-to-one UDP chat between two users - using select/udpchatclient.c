/*
one-to-one UDP chat between two users (one on server and the other on client) using select()

A IPv4 UDP client with the following command line usage 
udpchatclient servername [svrport] 
the svrport is optional which defaults to IPv4 port 9191.  For example, one can invoke the program with 
./udpchatclient localhost 

This is the client side of the one-to-one line-based chat via UDP.  
The client sends and receives messages line by line. 
After a user enters a message line (terminated with newline) from stdin of the client, the client sends this message line to the server. 
A user can enter multiple message lines from stdin (where the client sends line by line to the server) without receiving any message line from the server. 
The client also immediately displays any message line from the server to stdout of the client. 
The server can send multiple message lines to the client while the client does not send any message line over to the server.  
It is possible to have interleaved message lines, for example, the client displays a message line from the server while the user (on the client) is in the middle of entering a message line. 
•	The client looks up the endpoint info by calling getaddrinfo(3), before the client calls socket(2). 
•	The client is a single process without any pthread.  The client must use select(), sendto(), and recvfrom(). 
•	The client prefixes each message line from the server to stdout with client local time and server host name (or IP), for example: 
Sat Aug  4 14:54:33 2018, <serverName or serverIP>: howdy from server 
•	The client displays error string for any system call to stderr of the client. 
•	This is a one-to-one chat and therefore the UDP server only chats with the first UDP chat client.  
  The UDP chat server sends a special message line “<<Server Busy>>\n” to any subsequent incoming UDP chat clients. 
  Upon receiving such a message line, the client displays this message line and then terminates the client process. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/select.h>
#define LEN 5400
#define PRINT
char serverbuff[20];

int main (int argc, char* argv[])
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int msock, s,c;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM ; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */
	s = getaddrinfo(argv[1],argv[2] , &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		msock = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		if (msock == -1)
			continue;
		PRINT("\nsocket created");

		break;
	}

	fd_set rfds;
	fd_set afds;
	int nfds, max;
	if(STDIN_FILENO > msock)
		max = STDIN_FILENO;
	else
		max = msock;
	nfds = max + 1;
	FD_ZERO(&afds);
	FD_ZERO(&rfds);
	FD_SET(msock, &afds);
	FD_SET(STDIN_FILENO, &afds);
	time_t t;

	char *request = malloc(LEN * sizeof(char));
	long unsigned int req_size;
	char *buffer = malloc(LEN * sizeof(char));
	struct sockaddr_in serv_addr;
	int point = sizeof(serv_addr);
	while(1)
	{
		PRINT("\nEntered in while loop");
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
			perror("\n Select error");
		if(FD_ISSET(msock, &rfds))
		{
			PRINT("\nEntered in socket");
			PRINT("\nEntered in socket");
			int r;
			t = time(NULL);
			memset(buffer, 0, LEN);
			r = recvfrom(msock, buffer, LEN, 0,(struct sockaddr *)&serv_addr, &point);
			if (r == -1)
			{
				perror("read error");
				return -1;
			}
			if(strcmp(buffer ,"Server busy") == 0)
			{
				printf("\n%s\n",buffer);
				break;
			}
			printf("%s, ", strtok(ctime(&t), "\n"));
			const char *d;
			d = inet_ntop(AF_INET, (const void *)result->ai_addr , serverbuff , sizeof(serverbuff));
			printf("%s: ", serverbuff );
			printf("%s\n",  buffer);
		}

		if(FD_ISSET(STDIN_FILENO, &rfds))
		{
			PRINT("\nEntered in STDIN");
			int w;
			memset(request, 0, LEN);
			getline(&request,&req_size, stdin);
			PRINT("\nValue entered : %s", request );
			w = sendto(msock,request, LEN,0, result->ai_addr, rp->ai_addrlen);
			if (w == -1)
			{
				 perror("write error");
				 return -1;
			}
			PRINT("\nwritting\n");
		}
	}

	c = close(msock);
	if (c == -1)
		{
			 perror("write error");
			 return -1;
		}
	PRINT("\nCLosed\n");
	return 0;
}




