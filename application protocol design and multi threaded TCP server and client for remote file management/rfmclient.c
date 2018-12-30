/*
A IPv4 TCP client with the following command line usage 
rfmclient servername [svrport] 
the svrport is optional which defaults to 9091.  For example, one can invoke the program with 
./rfmclient localhost 
After the client opens a single/persistent TCP connection to the specified server, 
the client gets into a loop which prompts a user to enter a command/request via stdin, 
sends to the server via this persistent connection, and displays the result/response from the server to stdout.  
Note that all requests from a given client are sent via this single/persistent TCP connection. 

Valid requests are as follows 
•	cat svrfilepath requests the server to return the content of the server-side file specified by the (server-side) absolute path svrfilepath.  For example, “cat /tmp/foo”. 
Success case: displays the file content (or any server-side error) to client-side stdout 
Error case: displays server-side error string 
•	rm svrfilepath requests the server to remove the server-side file specified by the (server-side) absolute path svrfilepath.  For example, “rm /tmp/bar”. 
Success case: displays nothing 
Error case: displays server-side error string 
•	stats 
requests the server to return current (server-side) statistics (defined later on server-side) and displays the result to client-side stdout. 
Success case: displays server-side statistics 
Error case: displays server-side error string 
•	exit 
informs the server that the client exits, and then the client calls shutdown() and terminate the process. 
Success case: displays nothing 
Error case: displays server-side error string 

If there is any server-side error while executing a client request (e.g., invalid file path, cannot access to the file, etc.),
the server sends the error string corresponding to the errno (strerror_r(3)) back to the client. 
The client display the error string, if any.  
•	The client looks up the endpoint info by calling getaddrinfo(3), before the client calls socket(2). 
•	The application protocol is designed in a way that the client can read each server-side response in its entirety via a single/persistent
  TCP connection. 
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
#include <assert.h>
#define PRINT

struct request_data
{
	char req_head[30];
	char req_body[100];
};
struct response_data
{
	char res_data[23000];
};

int main (int argc, char* argv[])
{
	char host[10];
	strcpy(host, argv[1]);

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	char *service = "9091";
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
	 char *request = malloc(300);
	 long unsigned int req_size;

	 struct response_data *rs_data = malloc(sizeof(struct response_data));
	 struct request_data *rq_data = malloc(sizeof(struct request_data));
	 while(1)
	 {
	 memset(request, 0, 300);
	 memset(rs_data, 0, sizeof(struct response_data));
	 memset(rq_data, 0, sizeof(struct request_data));
	 printf("\n");
	 getline(&request,&req_size, stdin);
	 PRINT("\nValue entered : %s", request );
	 strcpy(rq_data->req_head, strtok_r(request ," " ,&request));
	 strcpy(rq_data->req_body, strtok_r(NULL ," " , &request));
	 PRINT("\nCommand is %s", rq_data->req_head);
	 PRINT("\nPath is %s", rq_data->req_body);

	 int w;
	 w = write(sfd, rq_data, sizeof(struct request_data));
	 if (w == -1)
	  {
	  	perror("error");
	  	return -1;
	  }
	  int r;
	  int len = sizeof(struct response_data);
	  char *buffer ;
	  buffer = (char *)rs_data;
	  while((r = read(sfd,buffer,len)) > 0 )
	  {
		  buffer += r;
	   	  len -= r;
	  }
	  printf("\n%s\n", rs_data->res_data);
	  if(strcmp(rq_data->req_head, "exit")== 0)
		  break;
	 }
 	 return 0;
}





