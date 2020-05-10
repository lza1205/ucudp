
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <pthread.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <stdio.h>

#include "my_list.h"

#include "recv_pthread.h"

#define SERVER_PORT			8080		/* 服务器监听端口号 */

int sockfd;



#define __RECV_MAIN__
//#define __POLL_MAIN__
//#define __CREATE_LISTEN__


int init_socket(void)
{
    int sockfd;
    struct sockaddr_in serveraddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    return sockfd;
}



void *listen_phread(void *pdata)
{
	int ret;
	char buf[1204];
	struct sockaddr_in clientaddr;
	struct listen *_listen;
	_listen = (struct listen *)pdata;

	while(1)
	{
		ret = recv_from_listen(_listen, (struct sockaddr *)&clientaddr, buf, 1204, -1);
		if(ret == -1)
		{
			printf("%p recv is err \r\n", _listen);
			goto out;
		}else{
			printf("%p recv %d byte data is [%s]\r\n", _listen, ret, buf);
			if((ret = sendto(sockfd, buf, ret, 0, (struct sockaddr *)(&(_listen->addr)), 
									sizeof(struct sockaddr))) == -1)
			{
				perror("sendto :");
			}

			/*
			if 
			else if
			*/
			printf("sento [%s]\r\n", buf);
		}
	}
out:
	/* 关闭连接 */
	listen_close(_listen);
}

#ifdef __RECV_MAIN__
int main(int argc, char *argv[])
{
	struct listen *_listen;

	/* 初始化socket */
	sockfd = init_socket();	

	/*
		开始监听这个socket. 运行最大的连接数为10
		该函数类似于TCP协议中的 int listen(SOCKET sockfd, int backlog)
	*/
	server_listen(&sockfd, 10);

	while(1)
	{
		/* 获得一个连接。类似于TCP的
		int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
		*/
		_listen = server_accept();

		if(_listen == NULL){
			continue;
		}
		
		printf("new client \r\n");
		/* 启动一个线程，并且，由该线程去处理这个连接
			类似于TCP 的fork
		*/
		sleep(5);
		listen_pthread(_listen, listen_phread);
	}
}

#endif

/*
测试 poll 机制
*/
struct list_head poll_head_1, poll_head_2;

void *poll_pthread(void *pdata)
{	
	int ret;
	char buf[1204];
	struct sockaddr_in clientaddr;
	struct listen *_listen;

	struct list_head *poll_head = (struct list_head *)pdata;
	
	while(1)
	{
		ret = recv_from_listen_head(poll_head, &_listen, (struct sockaddr *)&clientaddr, buf, 1204, -1);
		if(ret == -1)
		{
			printf("%p recv is err \r\n", _listen);
		}else{
			printf("__ poll %p recv %d byte data is [%s]\r\n", _listen, ret, buf);
			if((ret = sendto(sockfd, buf, ret, 0, (struct sockaddr *)(&(_listen->addr)), 
									sizeof(struct sockaddr))) == -1)
			{
				perror("sendto :");
			}
			printf("sento [%s]\r\n", buf);
		}
	}
}

void create_poll_pthread(struct list_head *poll_head)
{	pthread_t recv_thread;
    pthread_attr_t recv_thread_attr;

	/* 创建子进程 */
	pthread_attr_init(&recv_thread_attr);		//初始化进程属性
	pthread_attr_setdetachstate(&recv_thread_attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&recv_thread, &recv_thread_attr, poll_pthread, poll_head) < 0)
	{
		perror("pthread_create");
		exit(1);
	}
}

#ifdef __POLL_MAIN__
int main(int argc, char *argv[])
{
	int poll_num = 0;
	struct listen *_listen;

	/* 初始化socket */
	sockfd = init_socket();	

	/*
		开始监听这个socket. 运行最大的连接数为10
		该函数类似于TCP协议中的 int listen(SOCKET sockfd, int backlog)
	*/
	server_listen(&sockfd, 10);

	/* 初始化这个poll 机制 */
	listen_head_init(&poll_head_1);
	listen_head_init(&poll_head_2);
	
	while(1)
	{
		/* 获得一个连接。类似于TCP的
		int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
		*/
		_listen = server_accept();

		if(_listen == NULL){
			continue;
		}
		
		printf("new client \r\n");

		if(poll_num < 5)
		{
			poll_num ++;
			listen_add(&poll_head_1, _listen);
			create_poll_pthread(&poll_head_1);
		}else if(poll_num < 7){
			poll_num ++;
			listen_add(&poll_head_2, _listen);
			create_poll_pthread(&poll_head_2);
		}else{
			listen_pthread(_listen, listen_phread);
		}
		
	}
}
#endif


#ifdef __CREATE_LISTEN__

int main(void)
{
	listen_t *listen;
	struct sockaddr_in addr;
	int ret;
	char buf[1204];
	struct sockaddr_in clientaddr;

	/* 初始化socket */
	sockfd = init_socket();	

	/*
		开始监听这个socket. 运行最大的连接数为10
		该函数类似于TCP协议中的 int listen(SOCKET sockfd, int backlog)
	*/
	server_listen(&sockfd, 10);

	/* 创建一个套接字。用来监听具体某个客户端 */
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("112.74.197.88");
    addr.sin_port = htons(8008);
	
	listen = create_listen((struct sockaddr *)&addr);

	while(1)
	{
		ret = recv_from_listen(listen, (struct sockaddr *)&clientaddr, buf, 1204, -1);
		if(ret == -1)
		{
			printf("%p recv is err \r\n", listen);
		}else{
			printf("%p recv %d byte data is [%s]\r\n", listen, ret, buf);
			if((ret = sendto(sockfd, buf, ret, 0, (struct sockaddr *)(&(listen->addr)), 
									sizeof(struct sockaddr))) == -1)
			{
				perror("sendto :");
			}
			printf("sento [%s]\r\n", buf);
		}
	}
	
}

#endif


