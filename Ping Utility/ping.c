/*
Minimum command-line ping utility using raw socket to send out ICMP echo request.  
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>

#define PRINT
#define LEN 30

int sfd;
struct sockaddr_in *result;
struct timeval start;
int msg_count = 0;
int length =0;

unsigned short checksum(const unsigned short *addr, int length)
{
	int l = length;
	const unsigned short *w = addr;
	unsigned short ans = 0;
	int sum = 0;

	while (l > 1)  {
		sum += *w++;
		l -= 2;
	}
	if (l == 1)
	{
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}


void send_packet()
{
	struct icmp *hdr_sent = malloc(sizeof(struct icmp));
	int datalen = 20;
    hdr_sent->icmp_type = ICMP_ECHO;
    hdr_sent->icmp_hun.ih_idseq.icd_id = getpid();
    hdr_sent->icmp_code = 0;
    hdr_sent->icmp_hun.ih_idseq.icd_seq = ++msg_count;

    memset(hdr_sent->icmp_data, 0xa5, datalen);
    gettimeofday((struct timeval *)hdr_sent->icmp_data, NULL);
    PRINT("\nInside send about to send\n");
    length = 8 + datalen;
    hdr_sent->icmp_cksum = checksum((u_short *) hdr_sent, length);
    if(sendto(sfd, hdr_sent, length, 0, (struct sockaddr *)result, sizeof(struct sockaddr)) <= 0)
    {
    	perror("\nPacket sending failed");
    	return;
    }
    else
    	PRINT("\nSend success with seq %d", hdr_sent->icmp_hun.ih_idseq.icd_seq);
    alarm(1);
}

void rcv_packet()
{
	char *hdr_rcv = malloc(sizeof(char) * LEN);
	PRINT("\nsize of icmp %ld\n ", sizeof(*hdr_rcv));
	struct sockaddr_in recv;
	unsigned int len = sizeof(recv);
	struct timeval stop;
	memset(&recv, 0, sizeof(struct sockaddr_in));
	memset(hdr_rcv, 0, LEN);
	struct iphdr *ip =  malloc(sizeof(struct iphdr));
	struct icmp *icmp = malloc(sizeof(struct icmp));
	memset(ip, 0, sizeof(struct iphdr));
	memset(icmp, 0, sizeof(struct icmp));
	PRINT("\n Memst success");
	int read;
	PRINT("\ninside rcv about to recv\n");
	read = recvfrom(sfd, hdr_rcv, LEN , 0, (struct sockaddr *)&recv, &len);
	if(read < 0)
	{
		perror("\nPacket receiving failed");
		return;
	}
	gettimeofday(&stop, NULL);
   	char buff1[INET_ADDRSTRLEN];
    const char *d;
    d = inet_ntop(AF_INET, &recv.sin_addr.s_addr , buff1 , INET_ADDRSTRLEN);
    if (d == NULL)
    {
    	printf("\ncouldnot convert address");
    	return;
    }
    PRINT("\n Sender address converted to text %s", buff1);
    memcpy(ip, hdr_rcv, sizeof(struct iphdr));
    memcpy(icmp, hdr_rcv + sizeof(struct iphdr), (read - sizeof(struct iphdr)));
    if(icmp->icmp_hun.ih_idseq.icd_id == getpid())
    {
    	printf("%d bytes", read);
    	printf("  from: %s", buff1);
    	printf("  ttl %d", ip->ttl);
    	printf("  icmp_seq = %d", icmp->icmp_hun.ih_idseq.icd_seq);
    	printf("  identifier = %d ", icmp->icmp_hun.ih_idseq.icd_id);
    	printf("  time = %lu microseconds\n", stop.tv_usec - atoi(icmp->icmp_dun.id_data));
    }

}

void alarm_handler()
{
    alarm(1);
    send_packet();// recurring alarm
}

int main(int argc, char *argv[])
{
	result = malloc(sizeof(struct sockaddr_in));
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sfd == -1)
	{
		perror("socket error");
		return -1;
	}
	PRINT("\nSocket created");
	char *source_addr = malloc (500);
	/*strcpy(source_addr, argv[1]);
	PRINT("\n source addr is %s", source_addr);*/
	result->sin_family = AF_INET;
	result->sin_port = htons(0);

	struct hostent *hp = gethostbyname(argv[1]);
	unsigned int i =0;

	while(hp->h_addr_list[i] != NULL)
	{
		strcpy(source_addr, inet_ntoa(*(struct in_addr*)(hp->h_addr_list[i])));
		PRINT("source adrr %s", source_addr);
		if(strlen(source_addr)!= 0)
			break;
	}

	int n;
	n = inet_pton(AF_INET, source_addr, &result->sin_addr.s_addr );

	if (n == 1)
		PRINT("\nAddress converted to binary");
	else if(n == 0)
		PRINT("\nAddress conversion error");
	PRINT("\n address converted is %s", inet_ntoa(result->sin_addr));
	PRINT("\n ");

	signal(SIGALRM, alarm_handler);
    alarm(1);
    while(1)
    {
        PRINT("\n About to go to rcv function\n");
    	rcv_packet();

        pause();
    }
    return 0;
}
