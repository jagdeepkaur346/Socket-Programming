/*
one-to-one UDP chat between two users (one on server and the other on client) using select(). 

UDP chat server with multi-users.  Now the server can chat with multiple clients.  
Any client can connect to the server at any time and any client can exit and therefore leaving the chat session at any time.  
Any message line entered by any user (on server or client) is broadcast to all other current users (regardless on server or client). 
No need to remember message history.  
The server is still a single process without any pthread.  The server must use select(), sendto(), and recvfrom(). 
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

#define list_len 50
#define PRINT
#define LEN 5400
char clientip[20];
uint16_t clientport;
char old_clientIP[40];
uint16_t old_clientPORT;
struct sockaddr_in list[list_len];
int count = 0;

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
    		int r;
    		time_t mytime = time(NULL);
    		char * time_str = ctime(&mytime);
    		time_str[strlen(time_str)-1] = '\0';
    		memset(buffer, 0, LEN);
    		int point = sizeof(struct sockaddr);
    		r = recvfrom(msock, buffer, LEN, 0, (struct sockaddr*)&result, &point);
    		if (r == -1)
    		{
    			 perror("read error");
    		     return -1;
    		}
    		PRINT("\nreading");

      		int already_exist = 0;
      		int b;
      		for(b = 0; b < count ; b++ )
      		{
      			if(memcmp(&list[b],&result , sizeof(struct sockaddr_in)) == 0)
      			{
      				already_exist= 1;
      				break;
      			}
      		}
      		if(already_exist != 1)
      		{
      			list[count]=result;
      			count += 1;
      		}
        	int a;
        	for(a= 0 ; a < count ; a++)
        	{
                int w;
                if(memcmp(&list[a],&result , sizeof(struct sockaddr_in)) != 0)
                	w = sendto(msock,buffer, LEN,0,(struct sockaddr *)&list[a], sizeof(struct sockaddr));
                if (w == -1)
                {
                     perror("write error");
                     return -1;
                 }
        	}
            const char *d;
            d = inet_ntop(AF_INET, &result.sin_addr , clientip , sizeof(clientip));
            clientport = ntohs(result.sin_port);
            PRINT("\n old clientip is %s", old_clientIP);
            PRINT("\n old port is %d", old_clientPORT);

       		printf("\n%s,", time_str);
        	printf(" %s:", clientip);
            printf(" %s\n",  buffer);
            memset(&result2, 0, sizeof(result2));
            memcpy(&result2,&result , sizeof(result2));
            strcpy(old_clientIP , clientip);
           	old_clientPORT = clientport;
    	}

    	if(FD_ISSET(STDIN_FILENO, &rfds))
    	{
            PRINT("\nEntered in STDIN");
    		int w;
            memset(request, 0, LEN);
            getline(&request,&req_size, stdin);
            PRINT("\nValue entered : %s", request);

      		int a;
        	for(a= 0 ; a < count; a++)
        	{
                int w;
                w = sendto(msock,request, LEN,0,(struct sockaddr *)&list[a], sizeof(struct sockaddr));
                if (w == -1)
                {
                     perror("write error");
                     return -1;
                 }
        	}
            PRINT("\nwritting\n");
          }
    }
    return 0;
}





