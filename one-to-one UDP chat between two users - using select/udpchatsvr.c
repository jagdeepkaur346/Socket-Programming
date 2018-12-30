/*
one-to-one UDP chat between two users (one on server and the other on client) using select(). 

A IPv4 UDP chat server with the following usage 
udpchatsvr [svrport] 
where svrport is optional which defaults to IPv4 port 9191.

This is the server side of the one-to-one line-based chat via UDP.  The server sends and receives messages line by line.  
After a user enters a message line (terminated with newline) from stdin of the server, the server sends this message line to the client (if exists). 
A user can enter multiple message lines from stdin (where the server sends line by line to the client) without receiving any message line from the client.  
The server also immediately displays any message line from the client to stdout of the server. 
The client can send multiple message lines to the server while the server does not send any message line over to the client. 
It is possible to have interleaved message lines, for example, the server displays a message line from the client while the user (on the server) is in the middle of entering a message line. 
•	The server is a single process without any pthread.  The server must use select(), sendto(), and recvfrom(). 
•	The server prefixes each message line from the client to stdout with server local time and client host name (or IP), for example: 
  Sat Aug  4 14:54:33 2018, <clientName or clientIP>: hello from client 
•	The server displays error string for any system call to stderr of the server. 
•	This is a one-to-one chat and therefore the UDP server only chats with the first UDP chat client.  
  The UDP chat server sends a special message line “<<Server Busy>>\n” to any subsequent incoming clients. 
  Upon receiving this message line, any client displays the message line and then terminates the client process. 
 
The server can differentiate two clients because each client has a unique combination of client IP and client port. 

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
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/select.h>

#define PRINT
#define LEN 5400
char clientip[20];
uint16_t clientport;
char old_clientIP[40];
uint16_t old_clientPORT;

int main (int argc, char* argv[])
{
    int msock, b, s_sock;
    struct sockaddr_in result, result2;
    msock = socket(AF_INET, SOCK_DGRAM, 0);
    if(msock == -1)
    {
    	perror("Socket not created\n");
    	return -1;
    }
    PRINT("\nMaster Socket created");
    int server_port = 9191;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int c;
    b = bind(msock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (b == -1)
    {
	perror("\nbind error");
    	return -1;
    }
    PRINT("\nBinded");

    fd_set rfds;
    fd_set afds;
    int nfds,max;
    if(STDIN_FILENO > msock)
    	max = STDIN_FILENO;
    else
    	max = msock;
    nfds = max  +1 ;
    FD_ZERO(&afds);
    FD_ZERO(&rfds);
    FD_SET(msock, &afds);
    FD_SET(STDIN_FILENO, &afds);

    char *request = malloc(LEN * sizeof(char));
    long unsigned int req_size;
    strcpy(old_clientIP, "0");
	char *buffer = malloc(LEN * sizeof(char));

    while(1)
    {
    	memcpy(&rfds, &afds, sizeof(rfds));
    	if(select(nfds, &rfds,(fd_set *)0 , (fd_set *)0, (struct timeval *)0 ) < 0)
    		perror("\nSElect error");
    	if(FD_ISSET(msock, &rfds))
    	{
    		PRINT("\nEntered in socket");
    		int r;
    		time_t mytime = time(NULL);
    		char * time_str = ctime(&mytime);
    		time_str[strlen(time_str)-1] = '\0';
    		int point = sizeof(struct sockaddr);
    		r = recvfrom(msock, buffer, LEN, 0, (struct sockaddr*)&result, &point);
    		if (r == -1)
    		{
    			 perror("read error");
    		     return -1;
    		}
    		PRINT("\nreading");
            const char *d;
            d = inet_ntop(AF_INET, &result.sin_addr , clientip , sizeof(clientip));
            clientport = ntohs(result.sin_port);
            PRINT("\n old clientip is %s", old_clientIP);
            PRINT("\n old port is %d", old_clientPORT);

       		if(((strcmp(old_clientIP, clientip) != 0) || (old_clientPORT != clientport) )&& (strcmp(old_clientIP,"0") != 0))
            	{
       				PRINT("\n It is new client");
       				char exit[5400];
            		strcpy(exit , "Server busy");
            		sendto(msock, exit ,sizeof(exit),0,(struct sockaddr *)&result, sizeof(struct sockaddr));
            	}
       		else
       		{
       			printf("\n%s,", time_str);
        		printf(" %s:", clientip);
               	printf(" %s\n",  buffer);
               	memset(&result2, 0, sizeof(result2));
               	memcpy(&result2,&result , sizeof(result2));
           		strcpy(old_clientIP , clientip);
           		old_clientPORT = clientport;
       		}
    	}

    	if(FD_ISSET(STDIN_FILENO, &rfds))
    	{
            PRINT("\nEntered in STDIN");
    		int w;
            memset(request, 0, LEN);
            getline(&request,&req_size, stdin);
            PRINT("\nValue entered : %s", request);
            w = sendto(msock,request,LEN,0,(struct sockaddr *)&result2, sizeof(struct sockaddr));
            if (w == -1)
            {
                   perror("write error");
                   return -1;
            }
            PRINT("\nwritting\n");
          }
    }
    return 0;
}


