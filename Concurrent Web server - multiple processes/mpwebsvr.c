/*
A concurrent Web server (listening only on port 80 on IPv4, and supporting only HTTP/1.0) based on multiple processes.
Any client only sends in exactly one HTTP request. 
Whenever a HTTP request comes in, the master web server creates a new child process to handle the client. 
•	The HTTP response line is always “HTTP/1.0 200 OK” 
•	The HTTP response headers are always 
ClientIP: <client-ip> 
ClientPort: <client-port> 
Replace <client-ip> and <client-port> with the actual values. 

After the client connects to a server, the server can programmatically identify the client-ip and client-port (ephemeral port). 
•	The HTTP response body is the entire HTTP request (including method, headers, and, if any, body) wrapped in HTML. 
<!DOCTYPE html> 
<html><body><blockquote> 
...the entire http request (method, headers, body) 
</blockquote></body></html> 
•	To simplify the code, no HTTP encoding is needed. 

The master web server  
•	Prints out “mpwebsvr <YourName> ” to stdout only once. 
A web child process 
• 	For the incoming client, prints the followings to stdout: server-pid = <pid>, client-ip = <client-ip>, client-port = <client-port> 
to stdout, sends out HTTP response, always calls “sleep(30);” to emulate processing time, close slave socket, and then exits. 
Replace <pid>, <client-ip>, and <client-port> with appropriate values, respectively.
A process identifies its own process ID (pid) by calling getpid().  One can adjust the value 30 in “sleep(30);” if necessary.
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

#define PRINT 

int id;
void reaper(int);
int	child(int fd);
char clientbuff[20];
char clientport[20];
int count = 15000;
char server_pid[20];

int main (int argc, char* argv[])
{
    printf("webclient-post Jagdeep kaur \n");

    int msock, b, l, s_sock;
    msock = socket(AF_INET, SOCK_STREAM, 0);
    if(msock == -1)
    {
    	perror("Socket not created\n");
    	return -1;
    }
    PRINT("\nMaster Socket created");
    int server_port = 80;
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
    int listen_queue = 10;   
    l = listen(msock, listen_queue);
    if (l == -1)
    {
	perror("\nlisten error");
    	return -1;
    }
    PRINT("\nlistening");

    (void) signal(SIGCHLD, reaper);
    struct sockaddr_in fsin;

     memset(&fsin, 0, sizeof(fsin));
     int alen;
     alen = sizeof(fsin);

    while(1)
    {
        s_sock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if (s_sock < 0)
        {
        	if (errno == EINTR)
        		continue;
        	perror("\nAccept error");
        }
        else
        	PRINT("\nAccepted");
        const char *d;
        d = inet_ntop(AF_INET, (const void *)&fsin.sin_addr , clientbuff , sizeof(clientbuff));
        PRINT("\nclient ip addr %s", clientbuff);
        
        sprintf(clientport , "%d" , ntohs(fsin.sin_port));
        PRINT("\nclient port number %d", fsin.sin_port);
        id = getpid();
        sprintf(server_pid, "%d" , id);
        switch(fork())
        {
        default: 
        	
            (void) close(s_sock);
            break;
        case 0:
        	(void) close(msock);
        	
        	exit(child(s_sock));       	        
        case -1:
        	perror("\nfork error:");
    	}
    }

    return 0;
}

int
child(int fd)
{	
	char *buf11 = "\nServer-pid = ";
	char *buf12 = server_pid ;
	char *buf13 = ", Client-ip = ";
	char *buf14 = clientbuff;
	char *buf15 = ", Client-port = ";
	char *buf16 = clientport;
	char *buf1 = malloc( strlen(buf11) + strlen(buf12) + strlen(buf13) + strlen(buf14) + strlen(buf15) + strlen(buf16) + 1);
	strcpy(buf1 , buf11);
	strcat(buf1 , buf12);
	strcat(buf1 , buf13);
	strcat(buf1 , buf14);
	strcat(buf1 , buf15);
	strcat(buf1 , buf16);
	printf("\n\n%s", buf1);
	int r;
	int size_buf2 = 15000;
	char *buf2 = malloc(sizeof(char) * size_buf2);
	char *final_request = buf2;
	while((r = read(fd , buf2, size_buf2))>0)
	{
		buf2 += r;
		size_buf2 -= r;
	}
	buf2++;
	*buf2 = '\0';
	PRINT("\n\n\n%s", final_request);
	char *buf17 = "HTTP/1.0 200 OK";
	char *buf18 = "\nClientIP: ";
	char *buf19 = "\nClientPort: ";
	char *buf20 = "\n\n<!DOCTYPE html> \n<html><body><blockquote>\n";
	char *buf21 = "\n</blockquote></body></html>";
	char *buf3 = malloc( strlen(buf17) + strlen(buf18) + strlen(buf14) + strlen(buf19) + strlen(buf16) + strlen(buf20) + strlen(buf21) +strlen(buf2) +1);
	strcpy(buf3 , buf17);
	strcat(buf3 , buf18);
	strcat(buf3 , buf14);
	strcat(buf3 , buf19);
	strcat(buf3 , buf16);
	strcat(buf3 , buf20);
	strcat(buf3 , final_request);
	strcat(buf3 , buf21);
	
	write(fd , buf3 , count);
	sleep(30);	
	return 0;
}

void
reaper(int sig)
{
	int	status;
	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}

