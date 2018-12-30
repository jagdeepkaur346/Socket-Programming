/*
a IPv4 TCP concurrent remote file management server based on pthread with the following usage 
mtrfmsvr [svrport] 
where svrport is optional which defaults to port 9091. 
The stateless server creates one pthread per client.  
Whenever a client connects to the server, the server creates a detached pthread that handles multiple requests from the same client by using the same TCP connection. 
Valid commands are specified on the client already.  The pthread exits when it receives the “exit” command from the client. 

The master server 
•	Listens on IPv4 port 9091 by default (unless the server was started with the optional svrport argument) 
•	The server uses a pthread mutex variable to protect the following global statistics in a data structure which can be updated by each pthread: concurrent connections: +1 when a pthread starts; -1 when a pthread exits completed connections: +1 when a pthread exits 
         number of commands received across all clients: +1 when a client request is received 
Each pthread 
•	Prints “thread <pthreadID>: started” to stdout when it is started.  Replace <pthreadID> with the value returned from pthread_self(). 
•	Prints “thread <pthreadID>: exited” to stdout when it ends. 
•	Prints “thread <pthreadID>: <clientCmd>” to stdout.  
  Replace <clientCmd> with each client request received. 
  Prints “invalid command: <clientCmd>” to stdout for unrecognized command, if any 
•	Prints any error string to stderr for any system call failure (i.e., errno is not equals 0). 
  If these errors were caused by a client request, also sends the error string back to the client. 
•	Any API called by the server must be thread-safe (e.g., strerror_r(3)). 
•	The application protocol must be designed in a way that the server-side pthread can read each client-side request in its entirety via a single/persistent TCP connection. 
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
#include <assert.h>
#include <pthread.h>
#define PRINT

struct
{
	pthread_mutex_t	st_mutex;
	unsigned int	st_concurrent;
	unsigned int	st_complete;
	unsigned int    st_commands;
} stats;

struct request_data
{
    char req_head[30];
    char req_body[100];
};
struct response_data
{
    char res_data[23000];
};
int	server(int fd);

int main (int argc, char* argv[])
{
    int msock, b, l, ssock;
    msock = socket(AF_INET, SOCK_STREAM, 0);
    if(msock == -1)
    {
    	perror("Socket not created\n");
    	return -1;
    }
    PRINT("\nMaster Socket created");

    int option=1;
    setsockopt(msock, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));

    int server_port = 9091;
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
    struct sockaddr_in fsin;
    memset(&fsin, 0, sizeof(fsin));
    int alen;
    alen = sizeof(fsin);

    pthread_t th;
    pthread_attr_t ta;
    (void) pthread_attr_init(&ta);
    (void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
    (void) pthread_mutex_init(&stats.st_mutex, 0);
    while (1)
    {
    	alen = sizeof(fsin);
    	ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
    	if (ssock < 0)
    	{
    		if (errno == EINTR)
    			continue;
    		perror("\nAccept error");
    	 }
    	else
    		PRINT("\nAccepted");
    	if (pthread_create(&th, &ta, (void * (*)(void *))server,(void *)((long)ssock)) < 0)
    		perror("\nThread create error");
    }

    close(msock);
    return 0;
}

int server(int fd)
{
	int thread_id = pthread_self();
	struct request_data *rq_data = malloc(sizeof(struct request_data)) ;
	struct response_data *res_buf = malloc(sizeof(struct response_data));
	char *buf2;
	int r;
	printf("\nThread %d : started\n", thread_id);
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_concurrent++;
	(void) pthread_mutex_unlock(&stats.st_mutex);
	while(1)
	{
		int w;
		memset(res_buf,0 ,sizeof(struct response_data));
		memset(rq_data,0 ,sizeof(struct request_data));
		buf2 = (char *)rq_data;
		int size_buf2 = sizeof(struct request_data);
		while((r = read(fd , buf2, size_buf2))>0 && size_buf2 >0)
		{
			buf2 += r;
			size_buf2 -= r;
		}
		PRINT("\n Request received is %s",rq_data->req_head );
		(void) pthread_mutex_lock(&stats.st_mutex);
		stats.st_commands++;
		(void) pthread_mutex_unlock(&stats.st_mutex);
		if(strcmp(rq_data->req_head , "stats") != 0 && strcmp(rq_data->req_head , "cat") != 0 && strcmp(rq_data->req_head , "rm") != 0 && strcmp(rq_data->req_head , "exit") != 0 )
		{
			printf("\nThread %d : %s", thread_id , rq_data->req_head);
			printf("\n%s: Invalid command", rq_data->req_head);
			char *path = malloc(sizeof(struct request_data));
			strcpy(path, rq_data->req_head);
			strcat(path, " 2>&1");
			char q[sizeof(struct request_data)];
			strcpy(q, rq_data->req_head);
			strcat(q, ": Invalid command");
			strcpy(res_buf->res_data , q);
			write(fd , res_buf , sizeof(struct response_data));
			if(w == -1)
			{
				perror("\nWrite error");
				return -1;
			}
		}
		else if(strcmp(rq_data->req_head , "stats") == 0)
		{
			printf("\nThread %d : %s", thread_id , rq_data->req_head);
			char reply1[10], reply2[10], reply3[10];
			(void) pthread_mutex_lock(&stats.st_mutex);
			sprintf(reply1 , "%d" , stats.st_concurrent );
			strcpy(res_buf->res_data , "\nConcurrent: ");
			strcat(res_buf->res_data , reply1);
			sprintf(reply2 , "%d" , stats.st_commands );
			strcat(res_buf->res_data , "\nCommands: ");
			strcat(res_buf->res_data , reply2);
			sprintf(reply3, "%d" , stats.st_complete );
			strcat(res_buf->res_data , "\nComplete: ");
			strcat(res_buf->res_data , reply3);
			PRINT("\nresponse is  \n %s", res_buf->res_data);
			(void) pthread_mutex_unlock(&stats.st_mutex);
			write(fd , res_buf , sizeof(struct response_data));
			if(w == -1)
			{
				perror("\nWrite error");
				return -1;
			}
		}
		else
		{
			PRINT("\n it is a cat or rm or exit request");
			FILE *f;
			char *path = malloc(sizeof(struct request_data));
			strcpy(path, rq_data->req_head);
			if(strcmp(rq_data->req_head , "cat") == 0 || strcmp(rq_data->req_head , "rm") == 0)
			{
				strcat(path, " ");
				strcat(path, rq_data->req_body);
				printf("\nThread %d : %s", thread_id , path);
				strcat(path, " 2>&1");
			}
			else
				printf("\nThread %d : %s", thread_id , path);
			f = popen(path , "r");
			fread(res_buf->res_data , sizeof(struct response_data), 1, f);
			PRINT("\nresponse is  \n %s", res_buf->res_data);
			write(fd , res_buf , sizeof(struct response_data));
			if(w == -1)
			{
				perror("\nWrite error");
				return -1;
			}
			if(strcmp(rq_data->req_head , "exit") == 0)
				break;
			pclose(f);

		}
		printf("\n");
	}
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_concurrent--;
	stats.st_complete++;
	(void) pthread_mutex_unlock(&stats.st_mutex);
	close(fd);
	printf("\n\nThread %d : exited", thread_id);
	printf("\n");
	return 0;
}
