#include <stdio.h> 
#include <string.h> 
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "q_client.h"

#define MAXLINE 2*1024 
#define SERV_PORT 6666 
#define SERV_IP "127.0.0.1"

#define MY_CLIENT_NAME  "client1"

UINT32 g_client_no;

ALL_CLIENT_INFO g_allClientInfo;


int ClientSendRegist(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_REGISTER;
	message.header.clientNo = 0;
	memcpy(message.header.clientName, MY_CLIENT_NAME, NAME_LEN);
	memcpy(message.send_data, MY_CLIENT_NAME, sizeof(MY_CLIENT_NAME));
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendRegist:sendto error!\n");
		return ERROR;
	}
	printf("[client]ClientSendRegist ok\n");
	return OK;
}

int ClientSendUnRegist(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_UNREGISTER;
	message.header.clientNo = g_client_no;
	memcpy(message.header.clientName, MY_CLIENT_NAME, NAME_LEN);
	memcpy(message.send_data, MY_CLIENT_NAME, sizeof(MY_CLIENT_NAME));
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendUnRegist:sendto error!\n");
		return ERROR;
	}
	printf("[client]ClientSendUnRegist ok\n");
	return OK;
}

int ClientSendLogIn(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_LOG_IN;
	message.header.clientNo = 0;
	memcpy(message.header.clientName, MY_CLIENT_NAME, NAME_LEN);
	memcpy(message.send_data, MY_CLIENT_NAME, sizeof(MY_CLIENT_NAME));
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendLogIn:sendto error!\n");
		return ERROR;
	}
	printf("[client]ClientSendLogIn ok\n");
	return OK;
}

int ClientSendLogOut(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_LOG_OUT;
	message.header.clientNo = 0;
	memcpy(message.header.clientName, MY_CLIENT_NAME, NAME_LEN);
	memcpy(message.send_data, MY_CLIENT_NAME, sizeof(MY_CLIENT_NAME));
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendLogOut:sendto error!\n");
		return ERROR;
	}
	printf("[client]ClientSendLogOut ok\n");
	return OK;
}

int ClientSendGetAllInfo(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_GETALL_INFO;
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientRegist sendto error!\n");
		return ERROR;
	}
	printf("[client]send getAllInfo ok\n");
	return OK;
}

int ClientSendMessage(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_MESSAGE;
	message.header.clientNo = g_client_no;
	message.header.destNo = g_allClientInfo.client[0].clientNo;
	memcpy(message.header.clientName, MY_CLIENT_NAME, NAME_LEN);
	memcpy(message.send_data, "hello", sizeof("hello"));
	if(-1 == sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendMessage sendto error!\n");
	}
	else
	{
		printf("[client]ClientSendMessage sendto success:%s\n", message.send_data);
	}
}

void *ClientSendTask(void *arg)
{
	int sockfd = *(int *)arg;
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);

	int i = 0;
	while(1)
	{
		sleep(3);
		ClientSendMessage(sockfd, &servaddr);
		if(5 < i++)
		{
			break;
		}
	}
	ClientSendLogOut(sockfd, &servaddr);
	sleep(5);
	ClientSendUnRegist(sockfd, &servaddr);
}

int ProcessCmdGetAllInfo(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	int count = 0;
	CLIENT_INFO *result = NULL;
	ALL_CLIENT_INFO *clientInfo = NULL;
	int i =0;

	clientInfo = (ALL_CLIENT_INFO*)(recv_buf + sizeof(NET_CMD_HEADER));
	memcpy(&g_allClientInfo, clientInfo, sizeof(ALL_CLIENT_INFO));
	printf("[client]get all clientInfo:\n");
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		printf("[%d]clientNo:%d, clientName:%s, ip:%s, login:%d\n", i+1, g_allClientInfo.client[i].clientNo, g_allClientInfo.client[i].clientName, 
			inet_ntoa(g_allClientInfo.client[i].cliaddr.sin_addr), g_allClientInfo.client[i].login);
		if(0 == strcmp(MY_CLIENT_NAME, g_allClientInfo.client[i].clientName))
		{
			g_client_no = g_allClientInfo.client[i].clientNo;
		}
	}
	printf("[client]find my clientNo:%d\n", g_client_no);
	
	return OK;
}

int ProcessCmdMessage(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;
	char src_name[NAME_LEN] = {0};
	int i = 0;

	#if 0
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		if(g_allClientInfo.client[i].clientNo == recv_header->clientNo)
		{
			memcpy(src_name, g_allClientInfo.client[i].clientName, NAME_LEN);
			break;
		}
	}
	#endif
	
	printf("[client]recev message from [client%d-%s]:%s\n", recv_header->clientNo, recv_header->clientName, recv_buf+sizeof(NET_CMD_HEADER));
	
	return OK;
}

int ClientProcessMessage(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;

	if(!recv_buf || !cliaddr)
	{
		printf("[client]ClientProcessMessage param error!\n");
		return ERROR;
	}

	printf("[client]recev cmd:%d, data:%s\n", recv_header->netCmd, recv_buf+sizeof(NET_CMD_HEADER));

	if(CMD_GETALL_INFO == recv_header->netCmd)
	{
		//获取所有客户端信息
		ProcessCmdGetAllInfo(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_MESSAGE == recv_header->netCmd)
	{
		//消息
		ProcessCmdMessage(sockfd, recv_buf, recv_len, cliaddr);
	}

	return OK;
}


void *QclientTask(void *arg) 
{ 
	struct sockaddr_in servaddr, cliaddr; 
	socklen_t cliaddr_len; 
	int sockfd; 
	char recv_buf[MAXLINE]; 
	char str[INET_ADDRSTRLEN]; 
	int i; 
	int recv_len;
	pthread_t ntid; 

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT); 
 
	ClientSendRegist(sockfd, &servaddr);
	ClientSendLogIn(sockfd, &servaddr);
	ClientSendGetAllInfo(sockfd, &servaddr);
	//pthread_create(&ntid, NULL, ClientSendTask, &sockfd);
	
	while (1) 
	{ 
		cliaddr_len = sizeof(cliaddr); 
		recv_len = recvfrom(sockfd, recv_buf, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len); 
		if (recv_len == -1)
		{
			printf("[client]recvfrom error!\n"); 
		}
        else
		{
			ClientProcessMessage(sockfd, recv_buf, recv_len, &cliaddr);
		}
	} 
} 

int main(int argc, char **argv)
{
	pthread_t ntid; 
	if (0 != pthread_create(&ntid, NULL, QclientTask, NULL))
	{
		printf("create QserverTask fail!\n");
	}
	while(1)
	{
		pause();
	}
}

