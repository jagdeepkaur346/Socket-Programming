/*
A web client with the following command line usage 
webclient-post url 

For example, one can Invoke the program with 
./webclient-post http://localhost/ 
which sends a HTTP POST request to the specified URL.  
The web client then calls shutdown(), and prints (to stdout) the entire HTTP response (header and body), and then exits. 
•	The entire HTTP post request is as follows 
POST URL HTTP/1.0 
 
ClientIP = <client-ip> 
ClientPort = <client-port> 
 
URL is the command line argument.  There is no HTTP request headers.  But there are two lines in the HTTP request body. 
Replace <client-ip> and <client-port> with the actual values.  
After the client connects to a server, the client can programmatically identify the client-ip and client-port (ephemeral port). 
•	The client looks up the endpoint info by calling getaddrinfo(3), before the client calls socket(2). 
•	To simplify the code, the client only handles http on port 80.  Note the URL can be any valid URL based on http (without port). 
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
#define PRINT 

int main (int argc, char* argv[])
{
	int success = 0;
	char path[20];
	char host[20];
	if (sscanf(argv[1], "http://%99[^/]/%199[^\n]", host, path) == 2)
	{
		success = 1;/* http://hostname/page*/
	}
	else if (sscanf(argv[1], "http://%99[^/]/[^\n]", host) == 1)
	{
		success = 1;  /* http://hostname/ */
	}
	else if (sscanf(argv[1], "http://%99[^\n]", host) == 1)
	{
		success =1;  /* http://hostname */
	}
	PRINT("\n\nHost is %s  ", host);
	PRINT("\nPath is %s", path);

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	char *service = "http";
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM ;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(host,service , &hints, &result);
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
		PRINT("\nSocket created");
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			PRINT("\nConnection is established ");
			break;
		}                  /* Success */
		close(sfd);
	 }
	 if (rp == NULL)   
	 {
		 fprintf(stderr, "Could not connect\n");
		 exit(EXIT_FAILURE);
	 }
	 freeaddrinfo(result);           /* No longer needed */

	 struct sockaddr_in client_addr;
	 memset(&client_addr, 0, sizeof(struct sockaddr_in));
	 int c, size ;
	 size = sizeof(client_addr);
	 c= getsockname(sfd, (struct sockaddr *)&client_addr, &size);
	 	 
	 char buff1[INET_ADDRSTRLEN];	 
	 const char *d;
	 d = inet_ntop(AF_INET, (const void *)&client_addr.sin_addr , buff1 , sizeof(buff1));
	 PRINT("\nClient ip addr %s", buff1);	 	 
	 PRINT("\nClient port number %d", client_addr.sin_port);
	 
	 int p = ntohs(client_addr.sin_port);
	 char port[20];
	 sprintf(port, "%d" , p);

	 char *url1="POST ";
	 char *url2= argv[1];
 	 char *url3= " HTTP/1.0\r\n";
 	 char *url4= "\r\n";
 	 char *url5= "ClientIP = ";
 	 char *url6= buff1;
 	 char *url7="\r\n";
 	 char *url8="ClientPort = " ;
 	 char *url9 = port;
 	 char *url10 = "\r\n";
 	 char *url = malloc( strlen(url1)+ strlen (url2) + strlen (url3) + strlen(url4) + strlen(url5) + strlen(url6) + strlen(url7) + strlen(url8) +strlen(url9) + strlen(url10) + 1);
 	 strcpy(url ,url1);
 	 strcat(url, url2);
 	 strcat(url, url3);
 	 strcat(url, url4);
 	 strcat(url, url5);
 	 strcat(url, url6);
 	 strcat(url, url7);
 	 strcat(url, url8);
 	 strcat(url, url9);
 	 strcat(url, url10);
 	
 	 int w,r,m;
 	 w = write(sfd, url, strlen(url));
 	 if (w == -1)
 	 {
 		 perror("error");
 		 return -1;
 	 }
 	 PRINT("\nHTTP request is \n%s",url);
 	 shutdown(sfd, SHUT_WR);
 	 PRINT("\nWrite half is closed\n");

 	 int len = 15000;
 	 char *buffer = malloc(sizeof(char) * len);
 	 char *final = buffer;
 	 while((r = read (sfd, buffer,len))>0)
 	 {
 		 buffer += r;
 		 len -= r;
 	 }
 	 buffer++;
 	 *buffer = '\0';
 	 printf("\n\n%s\n", final);
 	 m= close(sfd);
 	 if (m == -1)
 	 {
 	 	perror("error");
 	 	return -1;
 	 }
 	 PRINT("\nSocket is closed\n");
 	 return 0;
}


