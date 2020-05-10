
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>



int init_socket(int port)
{
    int sockfd;
    struct sockaddr_in my_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    return sockfd;
}


int main(int argc, char *argv[])
{
	int sockfd;
	int addr_len;
	int numbytes;
	char buf[1024];
	struct sockaddr_in server_addr, recv_addr;
	
	if(argc != 2)
	{
		fprintf(stderr,"usage: talker hostname message\r\n");
		exit(1);
	}

	sockfd = init_socket(*(argv[1]));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = inet_addr("112.74.197.88");
	/* �ṹ����δ�õ�ȫ������ */
	bzero(&(server_addr.sin_zero), 8);

	addr_len = sizeof(server_addr);

//	while(1)
	int i;
	char send_buf[10];
	for(i = 0; i < 20; i++)
	{
		sprintf(send_buf, "%d", i);
		/* �������� */
		if((numbytes = sendto(sockfd, send_buf, strlen(send_buf), 0, 
			(struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1)
		{
			perror("sendto");
		}
#if 0
		/* �������� */
		if((numbytes = sendto(sockfd, argv[1], strlen(argv[1]), 0, 
			(struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1)
		{
			perror("sendto");
		}
		/* �������� */

		if((numbytes = recvfrom(sockfd, (char *)buf, 1024, 0, 
			(struct sockaddr *)&recv_addr, (socklen_t *)&addr_len)) == -1)
		{
			perror("sendto");
		}else{
			buf[numbytes] = 0;
			printf("recv server : [%s]\r\n", buf);
		}

		sleep(2);
#endif

	}
}


