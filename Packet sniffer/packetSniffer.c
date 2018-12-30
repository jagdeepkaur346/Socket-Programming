/*
Minimum command-line packet sniffer using pcap or libpcap library. 
Identify all network interfaces, and capture packets (incoming/outgoing) on a selected interface. 
Analyze the packets. (header info such as protocol, source, destination etc.) 
*/

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<net/ethernet.h>
#include<netinet/ip_icmp.h>
#include<netinet/udp.h>
#include<netinet/tcp.h>
#include<netinet/ip.h>

#define PRINT

FILE *file;
struct sockaddr_in source,dest;
void my_callback(u_char *args, const struct pcap_pkthdr *header,const u_char *packet);
void icmp_packet(const u_char *packet, int size);
void tcp_packet(const u_char *packet, int size);
void print_ip_header(const u_char * Buffer, int Size);
void udp_packet(const u_char *packet, int size);
int icmp=0, tcp =0, udp =0;

void print_ethernet_header(const u_char *packet, int size)
{
    struct ethhdr *eth = (struct ethhdr *)packet;

    fprintf(file , "\n");
    fprintf(file , " \nEthernet Header");
    fprintf(file , " \nDestination Address : %x-%x-%x-%x-%x-%x ", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5] );
    fprintf(file , " \nSource Address : %x-%x-%x-%x-%x-%x ", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    fprintf(file , " \nProtocol : %u \n",(unsigned short)eth->h_proto);
}

void print_ip_header(const u_char * packet, int size)
{
    print_ethernet_header(packet , size);

    unsigned short iplen;
    struct iphdr *ip = (struct iphdr *)(packet  + sizeof(struct ethhdr) );
    iplen =ip->ihl*4;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    fprintf(file , "\n");
    fprintf(file , "IP Header");
    fprintf(file , " \nVersion : %d",(unsigned int)ip->version);
    fprintf(file , " \nHeader Length : %d Bytes",((unsigned int)(ip->ihl))*4);
    fprintf(file , " \nType of Service : %d",(unsigned int)ip->tos);
    fprintf(file , " \nTotal Length : %d  Bytes(Size of Packet)",ntohs(ip->tot_len));
    fprintf(file , " \nIdentification : %d",ntohs(ip->id));
    fprintf(file , " \nTTL : %d",(unsigned int)ip->ttl);
    fprintf(file , " \nProtocol : %d",(unsigned int)ip->protocol);
    fprintf(file , " \nChecksum : %d",ntohs(ip->check));
    fprintf(file , " \nSource IP address: %s" , inet_ntoa(source.sin_addr) );
    fprintf(file , " \nDestination IP address: %s\n" , inet_ntoa(dest.sin_addr) );
}



void my_callback(u_char *args, const struct pcap_pkthdr *header,const u_char *packet)
{
	int size = header->len;
	struct iphdr *ip = (struct iphdr*)(packet + sizeof(struct ethhdr));

	if(ip->protocol == 1)
	{
		++icmp;
		icmp_packet(packet, size);
	}
	else if(ip->protocol == 6)
	{
		++tcp;
		tcp_packet(packet, size);
	}
	else if(ip->protocol == 17)
	{
		++udp;
		udp_packet(packet, size);
	}
}

void icmp_packet(const u_char *packet, int size)
{
    unsigned short iplen;

    struct iphdr *ip = (struct iphdr *)(packet  + sizeof(struct ethhdr));
    iplen = ip->ihl * 4;
    struct icmphdr *icmph = (struct icmphdr *)(packet + iplen  + sizeof(struct ethhdr));
    int header_size =  sizeof(struct ethhdr) + iplen + sizeof icmph;
    fprintf(file , "\n\n\nICMP Packet\n");
    print_ip_header(packet , size);
    fprintf(file , "\n");
    fprintf(file , "ICMP Header");
    fprintf(file , " \nType : %d",(unsigned int)(icmph->type));

    if((unsigned int)(icmph->type) == 11)
    {
        fprintf(file , " \n(TTL Expired)");
    }
    else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
    {
        fprintf(file , " \n(ICMP Echo Reply)");
    }

    fprintf(file , " \nCode : %d",(unsigned int)(icmph->code));
    fprintf(file , " \nChecksum : %d",ntohs(icmph->checksum));
    fprintf(file , "\n\n-----------------------------------------------------------------");

}

void tcp_packet(const u_char *packet, int size)
{
	unsigned short iplen;
	struct iphdr *ip = (struct iphdr *)( packet  + sizeof(struct ethhdr) );
	iplen = ip->ihl*4;

	struct tcphdr *tcph=(struct tcphdr*)(packet + iplen + sizeof(struct ethhdr));
	int header_size =  sizeof(struct ethhdr) + iplen + tcph->doff*4;

	fprintf(file , "\n\n\nTCP Packet\n");
	print_ip_header(packet,size);

	fprintf(file , "\n");
	fprintf(file , "TCP Header");
	fprintf(file , " \nSource Port : %u",ntohs(tcph->source));
	fprintf(file , " \nDestination Port : %u",ntohs(tcph->dest));
	fprintf(file , " \nSequence No. : %u",ntohl(tcph->seq));
	fprintf(file , " \nAcknowledge No. : %u",ntohl(tcph->ack_seq));
	fprintf(file , " \nHeader Length : %d BYTES" ,(unsigned int)tcph->doff*4);
	fprintf(file , " \nUrgent Flag : %d",(unsigned int)tcph->urg);
	fprintf(file , " \nAcknowledgement Flag : %d",(unsigned int)tcph->ack);
	fprintf(file , " \nPush Flag : %d",(unsigned int)tcph->psh);
	fprintf(file , " \nReset Flag : %d",(unsigned int)tcph->rst);
	fprintf(file , " \nSynchronise Flag : %d",(unsigned int)tcph->syn);
	fprintf(file , " \nFinish Flag : %d",(unsigned int)tcph->fin);
	fprintf(file , " \nWindow : %d",ntohs(tcph->window));
	fprintf(file , " \nChecksum : %d",ntohs(tcph->check));
	fprintf(file , " \nUrgent Pointer : %d",tcph->urg_ptr);
	fprintf(file , "\n\n----------------------------------------------------------------");
}

void udp_packet(const u_char *packet, int size)
{
    unsigned short iplen;

    struct iphdr *ip = (struct iphdr *)(packet +  sizeof(struct ethhdr));
    iplen = ip->ihl*4;

    struct udphdr *udph = (struct udphdr*)(packet + iplen  + sizeof(struct ethhdr));

    int header_size =  sizeof(struct ethhdr) + iplen + sizeof udph;

    fprintf(file , "\n\n\nUDP Packet\n");

    print_ip_header(packet,size);

    fprintf(file , "\nUDP Header");
    fprintf(file , " \nSource Port No. : %d" , ntohs(udph->source));
    fprintf(file , " \nDestination Port No. : %d" , ntohs(udph->dest));
    fprintf(file , " \nLength : %d" , ntohs(udph->len));
    fprintf(file , " \nChecksum : %d" , ntohs(udph->check));
    fprintf(file , "\n\n----------------------------------------------------------------");

}


int main(int argc, char *argv[])
{
	pcap_if_t *alldevsp , *device;
	int count =1,n;
	char *devname, errbuf[PCAP_ERRBUF_SIZE], devs[100][100];
	if( pcap_findalldevs( &alldevsp , errbuf) )
	{
	   printf("Error finding devices : %s" , errbuf);
	   return -1;
	}
	PRINT("\n Found all devices or interfaces");
	printf("\nAll network interfaces are :\n");
	for(device = alldevsp ; device != NULL ; device = device->next)
	{
	    printf("%d. %s \n" , count , device->name );
	    if(device->name != NULL)
	    {
	        strcpy(devs[count] , device->name);
	    }
	    count++;
	}

	printf("Select number for interface(to be sniffed) : ");
	scanf("%d" , &n);
	devname = devs[n];
	pcap_t *handle;
	handle = pcap_open_live(devname, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL)
	{
		fprintf(stderr, "Couldn't open interface %s: %s\n", devname, errbuf);
		return -1;
	}
	PRINT("\nOpened the selected interface");
	file=fopen("log.txt","w");
	if(file==NULL)
	{
	   printf("Unable to create file.");
	}

	pcap_loop(handle, -1, my_callback, NULL);
	printf("Sniffing completed\n");
	return(0);
}



