/*
A UDP client with the following command line usage 
udpclient host port msg 

where port can be any decimal port number or any service name (e.g., echo, daytime, etc.), and msg is the entire payload of UDP datagram. 
Invoke the program with 
./udpclient localhost echo “udp <YourName> ” 
which sends a UDP datagram to the ECHO server.  
For example, 
./udpclient localhost echo “udp demo 123” 

The UDP client then reads the response, outputs the following two lines to stdout 
the client’s current local datetime the UDP response length in bytes: the UDP response and then exits.  
Note the client’s current local datetime is not part of the original UDP request. 
•	The client looks up the server info by calling getaddrinfo(3), before the client calls socket(2). 
•	udpclient must programmatically determine the proper port number; it cannot be hard-coded to echo. 
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

#define LEN 14000
#define PRINT

int main (int argc, char* argv[])
{
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "\nCurrent local time and date: %s", asctime (timeinfo) );

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
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
		sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		if (sfd == -1)
			continue;
		PRINT("\nsocket created");
		break;
	}

	int w,r,c;
	char input[40];
	strcpy(input,argv[3]);	
	w = sendto(sfd,input,sizeof(input),0,result->ai_addr, sizeof(struct sockaddr));
	if (w == -1)
	{
		 perror("write error");
		 return -1;
	}
	PRINT("\nwritting\n");

	char *buffer = malloc(sizeof(char) * LEN);
	socklen_t *point = malloc(sizeof(struct sockaddr));
	r = recvfrom(sfd, buffer, LEN, 0,result->ai_addr, point);
	if (r == -1)
	{
		perror("read error");
		return -1;
	}
	buffer[r] = '\0';
	PRINT("\nreading");	
	printf("\nUDP datagram length is %d",r);
	printf("\n%s\n",  buffer);
	free(buffer);
	
	c = close(sfd);
	if (c == -1)
		{
			 perror("write error");
			 return -1;
		}
	PRINT("\nCLosed\n");
	return 0;
}
