/*
����1yPF_PACKET��?����?��tcp����??��? ����?����?����??2?D����a??1yD-����??��???������??a?����??������???�̨�3����㨹?��?��
*/

#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/wait.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/if.h>


char syn_1[] = {
0x00, 0x24, 0xE8, 0x75, 0xE5, 0x96, 0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x08, 0x00, 0x45, 0x00,
0x00, 0x3c, 0x2a, 0x5b, 0x40, 0x00, 0x40, 0x06, 0x8c, 0xd3, 0xc0, 0xa8, 0x01, 0x1e, 0xc0, 0xa8,
0x01, 0x1f, 0xdc, 0x16, 0x23, 0x82, 0xf4, 0x26, 0x93, 0x63, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
0x39, 0x08, 0x0d, 0xcc, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x04, 0x02, 0x08, 0x0a, 0x3d, 0x42,
0xb9, 0x38, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07
};

char syn_2[] = {
0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x08, 0x00, 0x45, 0x00,
0x00, 0x3c, 0x00, 0x00, 0x40, 0x00, 0x40, 0x06, 0xb7, 0x2e, 0xc0, 0xa8, 0x01, 0x1f, 0xc0, 0xa8,
0x01, 0x1e, 0x23, 0x82, 0xdc, 0x16, 0xb6, 0xb1, 0xea, 0x44, 0xf4, 0x26, 0x93, 0x64, 0xa0, 0x12,
0x38, 0x90, 0xf5, 0x9f, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x04, 0x02, 0x08, 0x0a, 0x3d, 0x43,
0x3a, 0x59, 0x3d, 0x42, 0xb9, 0x38, 0x01, 0x03, 0x03, 0x07
};

char syn_3[] = {
0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x08, 0x00, 0x45, 0x00,
0x00, 0x34, 0x2a, 0x5c, 0x40, 0x00, 0x40, 0x06, 0x8c, 0xda, 0xc0, 0xa8, 0x01, 0x1e, 0xc0, 0xa8,
0x01, 0x1f, 0xdc, 0x16, 0x23, 0x82, 0xf4, 0x26, 0x93, 0x64, 0xb6, 0xb1, 0xea, 0x45, 0x80, 0x10,
0x00, 0x73, 0x5c, 0x88, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x42, 0xb9, 0x39, 0x3d, 0x43,
0x3a, 0x59
};

char http_req[] = {
0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x08, 0x00, 0x45, 0x00,
0x00, 0x93, 0x2a, 0x5d, 0x40, 0x00, 0x40, 0x06, 0x8c, 0x7a, 0xc0, 0xa8, 0x01, 0x1e, 0xc0, 0xa8,
0x01, 0x1f, 0xdc, 0x16, 0x23, 0x82, 0xf4, 0x26, 0x93, 0x64, 0xb6, 0xb1, 0xea, 0x45, 0x80, 0x18,
0x00, 0x73, 0x0f, 0xfb, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x42, 0xb9, 0x39, 0x3d, 0x43,
0x3a, 0x59, 0x47, 0x45, 0x54, 0x20, 0x2f, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x54, 0x54,
0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a, 0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x31, 0x39, 0x32,
0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x35, 0x31, 0x3a, 0x39, 0x30, 0x39, 0x30, 0x0d, 0x0a,
0x55, 0x73, 0x65, 0x72, 0x2d, 0x41, 0x67, 0x65, 0x6e, 0x74, 0x3a, 0x20, 0x77, 0x65, 0x69, 0x67,
0x68, 0x74, 0x74, 0x70, 0x2f, 0x31, 0x2e, 0x34, 0x2e, 0x34, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x6e,
0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x63, 0x6c, 0x6f, 0x73, 0x65, 0x0d, 0x0a, 0x0d,
0x0a
};

char http_keep[] = {
0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x08, 0x00, 0x45, 0x00,
0x00, 0x34, 0xd0, 0x1c, 0x40, 0x00, 0x40, 0x06, 0xe7, 0x19, 0xc0, 0xa8, 0x01, 0x1f, 0xc0, 0xa8,
0x01, 0x1e, 0x23, 0x82, 0xdc, 0x16, 0xb6, 0xb1, 0xea, 0x45, 0xf4, 0x26, 0x93, 0xc3, 0x80, 0x10,
0x00, 0x72, 0x5c, 0x2a, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x43, 0x3a, 0x59, 0x3d, 0x42,
0xb9, 0x39
};

char http_resp[] = {
0x00, 0xe0, 0xd0, 0x1d, 0x40, 0x00, 0x40, 0x06, 0xe6, 0x6c, 0xc0, 0xa8, 0x01, 0x1f, 0xc0, 0xa8,
0x01, 0x1e, 0x23, 0x82, 0xdc, 0x16, 0xb6, 0xb1, 0xea, 0x45, 0xf4, 0x26, 0x93, 0xc3, 0x80, 0x18,
0x00, 0x72, 0x84, 0x60, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x43, 0x3a, 0x59, 0x3d, 0x42,
0xb9, 0x39, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f,
0x4b, 0x0d, 0x0a, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x3a, 0x20, 0x6e, 0x67, 0x69, 0x6e, 0x78,
0x2f, 0x31, 0x2e, 0x34, 0x2e, 0x34, 0x0d, 0x0a, 0x44, 0x61, 0x74, 0x65, 0x3a, 0x20, 0x46, 0x72,
0x69, 0x2c, 0x20, 0x31, 0x34, 0x20, 0x4d, 0x61, 0x72, 0x20, 0x32, 0x30, 0x31, 0x34, 0x20, 0x30,
0x32, 0x3a, 0x35, 0x39, 0x3a, 0x35, 0x38, 0x20, 0x47, 0x4d, 0x54, 0x0d, 0x0a, 0x43, 0x6f, 0x6e,
0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2f,
0x70, 0x6c, 0x61, 0x69, 0x6e, 0x0d, 0x0a, 0x54, 0x72, 0x61, 0x6e, 0x73, 0x66, 0x65, 0x72, 0x2d,
0x45, 0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x63, 0x68, 0x77, 0x6e, 0x6b, 0x65,
0x64, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x63,
0x6c, 0x6f, 0x73, 0x65, 0x0d, 0x0a, 0x0d, 0x0a, 0x63, 0x0d, 0x0a, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x0a, 0x0d, 0x0a, 0x30, 0x0d, 0x0a, 0x0d, 0x0a
};

char fin_1[] = 
{
0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x08, 0x00, 0x45, 0x00,
0x00, 0x34, 0xd0, 0x1e, 0x40, 0x00, 0x40, 0x06, 0xe7, 0x17, 0xc0, 0xa8, 0x01, 0x1f, 0xc0, 0xa8,
0x01, 0x1e, 0x23, 0x82, 0xdc, 0x16, 0xb6, 0xb1, 0xea, 0xf1, 0xf4, 0x26, 0x93, 0xc3, 0x80, 0x11,
0x00, 0x72, 0x5b, 0x7d, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x43, 0x3a, 0x59, 0x3d, 0x42,
0xb9, 0x39
};

char fin_2[] = {
0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x08, 0x00, 0x45, 0x00,
0x00, 0x34, 0x2a, 0x5f, 0x40, 0x00, 0x40, 0x06, 0x8c, 0xd7, 0xc0, 0xa8, 0x01, 0x1e, 0xc0, 0xa8,
0x01, 0x1f, 0xdc, 0x16, 0x23, 0x82, 0xf4, 0x26, 0x93, 0xc3, 0xb6, 0xb1, 0xea, 0xf2, 0x80, 0x11,
0x00, 0x7b, 0x5b, 0x73, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x42, 0xb9, 0x39, 0x3d, 0x43,
0x3a, 0x59	
};

char fin_3[] = {
0x00, 0x24, 0xE8, 0x79, 0x35, 0xED, 0x00, 0x24, 0xE8, 0x77, 0xE5, 0x96, 0x08, 0x00, 0x45, 0x00,
0x00, 0x34, 0xd0, 0x1f, 0x40, 0x00, 0x40, 0x06, 0xe7, 0x16, 0xc0, 0xa8, 0x01, 0x1f, 0xc0, 0xa8,
0x01, 0x1e, 0x23, 0x82, 0xdc, 0x16, 0xb6, 0xb1, 0xea, 0xf2, 0xf4, 0x26, 0x93, 0xc4, 0x80, 0x10,
0x00, 0x72, 0x5b, 0x7c, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x3d, 0x43, 0x3a, 0x59, 0x3d, 0x42,
0xb9, 0x39
};


void SendHttpPacket(int sockfd, struct sockaddr_ll* dest_ll)
{
	int port = 1300;
	int times = 0;
	while (times < 10000000)
	{
	times++;
	#if 0
		struct tcphdr* hdr_1 = (struct tcphdr*)(syn_1 + 20 + 14);
		hdr_1->source = ntohs(port); 
		if (sendto(sockfd, syn_1, sizeof(syn_1), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0) {
			perror("send error");
		}
    #endif

    #if 0
		struct tcphdr* hdr_2 = (struct tcphdr*)(syn_2 + 20 + 14);
		hdr_2->dest = ntohs(port); 
		if (sendto(sockfd, syn_2, sizeof(syn_2), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}
    
		struct tcphdr* hdr_3 = (struct tcphdr*)(syn_3 + 20 + 14);
		hdr_3->source = ntohs(port); 
		if (sendto(sockfd, syn_3, sizeof(syn_3), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}
    #endif

    #if 0
		struct tcphdr* hdr_4 = (struct tcphdr*)(http_req + 20 + 14);
		hdr_4->source = ntohs(port); 
		if (sendto(sockfd, http_req, sizeof(http_req), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}
#endif

    #endif

    #if 1

		struct tcphdr* hdr_5 = (struct tcphdr*)(http_resp + 20 + 14);
		hdr_5->dest = ntohs(port); 		
		if (sendto(sockfd, http_resp, sizeof(http_resp), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}
    #endif

    #if 0
		//sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)dest_ll, sizeof(struct sockaddr));

		struct tcphdr* hdr_6 = (struct tcphdr*)(fin_1 + 20 + 14);
		hdr_6->dest = ntohs(port); 
		if (sendto(sockfd, fin_1, sizeof(fin_1), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}

		struct tcphdr* hdr_7 = (struct tcphdr*)(fin_2 + 20 + 14);
		hdr_7->source = ntohs(port); 
		if (sendto(sockfd, fin_2, sizeof(fin_2), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}

		struct tcphdr* hdr_8 = (struct tcphdr*)(fin_3 + 20 + 14);
		hdr_8->dest = ntohs(port); 
		if (sendto(sockfd, fin_3, sizeof(fin_3), 0, (struct sockaddr*)(dest_ll), sizeof(*dest_ll)) <= 0)
		{
			perror("send error");
		}

		//port++;
		//if (port > 65534) port = 1300;
#endif

	}

}

int GetIntfaceIndex(int fd, const char* interface_name)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy (ifr.ifr_name, interface_name);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
    {
        return (-1);
    }
    return ifr.ifr_ifindex;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("please input the interface name \n");
        return 0;
    }

	char eth[10];
	strcpy(eth, argv[1]);
	
    int sockfd;        
	
	if((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        perror("socket error!"); 
        exit(1);
    }

	/*
	1.?�����?�� socket 
	2. bind ?��???��
	3. ?-?������?�� 1-7����??
	*/

	//
	unsigned char dmac[] = {0x00, 0x24, 0xE8, 0x77, 0x35, 0x12};
	
	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = GetIntfaceIndex(sockfd, eth);
	sll.sll_protocol = htons(ETH_P_IP);
	sll.sll_halen = 6;
	memcpy(sll.sll_addr, dmac, 6);

	SendHttpPacket(sockfd, &sll);

#if 0
	sll.sll_halen = 6;
    sll.sll_addr[0] = 0x00;
    sll.sll_addr[1] = 0x24;
    sll.sll_addr[2] = 0xe8;
    sll.sll_addr[3] = 0x79;
    sll.sll_addr[4] = 0x35;
    sll.sll_addr[5] = 0xed;
#endif

#if 0
	if (sendto(sockfd, syn_1, sizeof(syn_1), 0, (struct sockaddr*)&sll, sizeof(sll)) <= 0)
	{
		perror("send error");
	}
#endif

#if 0
		if (write(sockfd, syn_1, sizeof(syn_1)) <= 0)
		{
			perror("send error");
		}
#endif

	printf("After Send Over \n");
	
	return 0;
	
}