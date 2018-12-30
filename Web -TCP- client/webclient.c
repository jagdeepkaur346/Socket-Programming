/*
Write a web client with the following command line usage 
webclient url 

Invoke the program with 
./webclient http://localhost/ 

which sends a HTTP request “GET URL HTTP/1.0” to the specified URL (e.g., the local Apache http server), where URL is the command line 
argument.  The web client then calls shutdown(), prints (to stdout) the entire HTTP response (header and body), and then exits. 
•	The HTTP request does not have headers nor body. 
•	The client looks up the server info by calling getaddrinfo(3), before the client calls socket(2). 
•	To simplify the code, the client only handles http on port 80.  Note the URL is limited to any URL based on http (without port). 
•	Make sure HTTP response status is 200 or 3XX, not 4XX or 5XX. 

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
	printf("CMPE 207 HW1 tcp_client Jagdeep kaur 487");
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

	PRINT("\nHost is %s  ", host);


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
		PRINT("socket created");
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			PRINT("\nConnection is established ");
			break;
		}                  /* Success */

		close(sfd);
	 }
	 if (rp == NULL)   /* No address succeeded */
	 {
		 fprintf(stderr, "Could not connect\n");
		 exit(EXIT_FAILURE);
	 }
	 freeaddrinfo(result);           /* No longer needed */


	 char *url1="GET ";
	 char *url2= argv[1];
 	 char *url3= " HTTP/1.0\r\n";
 	 char *url4= "\r\n";
 	 char *url = malloc( strlen(url1)+ strlen (url2) + strlen (url3) + strlen(url4) + 1);
 	 strcpy(url ,url1);
 	 strcat(url, url2);
 	 strcat(url, url3);
 	 strcat(url, url4);
 	 int w,r,c;
 	 w = write(sfd, url, strlen(url));
 	 if (w == -1)
 	 {
 		 perror("error");
 		 return -1;
 	 }
 	 PRINT("\nHTTP request is %s",url);
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
 	 printf(" \n\n%s\n", final);
 	 c = close(sfd);
 	 if (w == -1)
 	 {
 	 	perror("error");
 	 	return -1;
 	 }
 	 PRINT("\nClosed\n");
 	 
 	 return 0;
}
