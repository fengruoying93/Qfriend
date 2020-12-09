#include <stdio.h> 
#include <string.h> 
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include "q_server.h"
#include "lstLib.h"

#define MAXLINE 2*1024 
#define SERV_PORT 6666 

LIST g_clientInfoList; //客户端信息存储链表

CLIENT_INFO_NODE_POOL g_clientInfoNodePool[MAX_CLIENT_NUM]; //静态内存链表节点池

CLIENT_INFO_NODE *getANodeFromPool(void)
{
	int i = 0;
	
	for(i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if(!g_clientInfoNodePool[i].used)
		{
			g_clientInfoNodePool[i].used = 1;
			return &g_clientInfoNodePool[i].infoNode;
		}
	}
	return NULL;
}

int freeANodeToPool(CLIENT_INFO_NODE *node)
{
	CLIENT_INFO_NODE_POOL *nodePoll = (CLIENT_INFO_NODE_POOL*)node;
	
	if(!node)
	{
		return ERROR;
	}
	memset(nodePoll, 0, sizeof(CLIENT_INFO_NODE_POOL));
	return OK;
}

void RamStorInitInfo(void)
{
	lstInit(&g_clientInfoList);
}

int RamStorListCount(void)
{
	return g_clientInfoList.count;
}

int RamStorAddNode(CLIENT_INFO_NODE *node)
{
	CLIENT_INFO_NODE *infoNode = NULL;

	if(!node)
	{
		return ERROR;
	}
	//infoNode = (CLIENT_INFO_NODE *)malloc(sizeof(CLIENT_INFO_NODE));
	infoNode = (CLIENT_INFO_NODE *)getANodeFromPool();
	if(!infoNode)
	{
		printf("malloc fail!\n");
		return ERROR;
	}
	memcpy(infoNode, node, sizeof(CLIENT_INFO_NODE));
	lstAdd(&g_clientInfoList, &infoNode->node);
	
	return OK;
}

int RamStorDeleteNode(char *clientName)
{
	CLIENT_INFO_NODE *pNode = NULL;
	if(!clientName)
	{
		printf("[server]RamStorDeleteNode param error\n");
		return ERROR;
	}
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(0 == strcmp(clientName, pNode->client.clientName))
		{
			break;
		}
	}
	if(NULL == pNode)
	{
		printf("[server]RamStorDeleteNode find none node!\n");
		return ERROR;
	}
	else
	{
		lstDelete(&g_clientInfoList, &pNode->node);
		//SAFE_FREE(pNode);
		freeANodeToPool(pNode);
	}
	return OK;
}


int ifClientRegisted(char *clientName)
{
	CLIENT_INFO_NODE *pNode = NULL;
	
	if(!clientName)
	{
		return FALSE;
	}
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(0 == strcmp(clientName, pNode->client.clientName))
		{
			return TRUE;
		}
	}
	return FALSE;
}

int ProcessCmdRegist(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf+sizeof(NET_CMD_HEADER);
	CLIENT_INFO_NODE *pNode = NULL;
	CLIENT_INFO_NODE clientNode;

	memset(&clientNode, 0, sizeof(CLIENT_INFO_NODE));
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(0 == strcmp(client_name, pNode->client.clientName))
		{
			printf("[server]client %s is already registered!\n", client_name);
			return ERROR;
		}
	}

	clientNode.client.clientNo = RamStorListCount()+1;
	memcpy(clientNode.client.clientName, recv_buf+sizeof(NET_CMD_HEADER), NAME_LEN);
	memcpy(&clientNode.client.cliaddr, cliaddr, sizeof(struct sockaddr_in));
	
	RamStorAddNode(&clientNode);
	
	printf("[server]insert regist -- clientNo:%d, clientName:%s, ip:%s, total_clientNum:%d\n", clientNode.client.clientNo, 
		clientNode.client.clientName, inet_ntoa(clientNode.client.cliaddr.sin_addr), RamStorListCount());
	
	return OK;
}

int ProcessCmdUnRegist(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf+sizeof(NET_CMD_HEADER);
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;

	if(OK == RamStorDeleteNode(client_name))
	{
		printf("[server]unRegist OK -- clientNo:%d, clientName:%s, total_clientNum:%d\n", recv_header->clientNo, 
			client_name, RamStorListCount());
	}
	
	return OK;
}


int ProcessCmdLogIn(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf+sizeof(NET_CMD_HEADER);
	CLIENT_INFO_NODE *pNode = NULL;

	if(!recv_buf || !cliaddr)
	{
		printf("[server]ProcessCmdLogIn param error!\n");
		return ERROR;
	} 
	
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(!strncmp(client_name, pNode->client.clientName, NAME_LEN))
		{
			printf("[server]ProcessCmdLogIn: find client%s!\n", client_name);
			pNode->client.login = 1;
			memcpy(&pNode->client.cliaddr, cliaddr, sizeof(struct sockaddr_in));
			printf("[server]client login success -- clientNo:%d, clientName:%s, ip:%s, total_clientNum:%d\n", pNode->client.clientNo, 
				pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr), RamStorListCount());
		}
	}
	
	return OK;
}

int ProcessCmdLogOut(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf+sizeof(NET_CMD_HEADER);
	CLIENT_INFO_NODE *pNode = NULL;

	if(!recv_buf || !cliaddr)
	{
		printf("[server]ProcessCmdLogOut param error!\n");
		return ERROR;
	} 
	
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(!strncmp(client_name, pNode->client.clientName, NAME_LEN))
		{
			printf("[server]ProcessCmdLogOut: find client%s!\n", client_name);
			pNode->client.login = 0;
			printf("[server]client logout success -- clientNo:%d, clientName:%s, ip:%s, total_clientNum:%d\n", pNode->client.clientNo, 
				pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr), RamStorListCount());
			memset(&pNode->client.cliaddr, 0, sizeof(struct sockaddr_in));
		}
	}
	
	return OK;
}


int ProcessCmdMessage(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;
	SEND_MESSAGE msg;
	UINT32 send_len = 0;
	CLIENT_INFO_NODE *pNode = NULL;

	bzero(&msg, sizeof(SEND_MESSAGE));
	send_len = (recv_len < sizeof(SEND_MESSAGE))?(recv_len):(sizeof(SEND_MESSAGE));
	memcpy(&msg, recv_buf, send_len);

	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(recv_header->destNo == pNode->client.clientNo)
		{
			//printf("[server]ProcessCmdMessage: find destClient%d:%s, ip:%s\n", pNode->client.clientNo, pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr));
			if (-1 == sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&pNode->client.cliaddr, sizeof(struct sockaddr_in)))
			{
				printf("[server]ProcessCmdMessage sendto error!%d:%s\n", errno, strerror(errno));
				return ERROR;
			}
			else
			{
				return OK;
			}
		}
	}
	printf("[server]ProcessCmdMessage: can not find dest client%d!\n", recv_header->destNo);
	return ERROR;
}

int ProcessCmdGetAllInfo(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	int count = 0;
	CLIENT_INFO_NODE *pNode = NULL;
	CLIENT_INFO *result = NULL;
	struct
	{
		NET_CMD_HEADER header;
		ALL_CLIENT_INFO clientInfo;
	}data;
	
	bzero(&data, sizeof(data));
	
	data.header.netCmd = CMD_GETALL_INFO;
	data.clientInfo.clientNum = RamStorListCount();
	result = data.clientInfo.client;
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		memcpy(result, &pNode->client, sizeof(CLIENT_INFO));
		result++;
		count++;
		if(count >= MAX_CLIENT_NUM)
			break;
	}
	
	if (-1 == sendto(sockfd, &data, sizeof(data), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in)))
	{
		printf("[server]ProcessCmdGetAllInfo sendto error!%d:%s\n", errno, strerror(errno));
	}
	
	return OK;
}

int ProcessClientRequest(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;

	if(!recv_buf || !cliaddr)
	{
		printf("[server]ProcessClientRequest param error!\n");
		return ERROR;
	}

	//printf("[server]recev cmd:%d, data:%s\n", recv_header->netCmd, recv_buf+sizeof(NET_CMD_HEADER));
	#if 0
	if(!ifClientRegisted(recv_header->clientName) && (CMD_REGISTER != recv_header->netCmd))
	{
		printf("[server]ProcessClientRequest:client no registed!\n");
		return ERROR;
	}
	#endif
	if(CMD_REGISTER == recv_header->netCmd)
	{
		//注册
		ProcessCmdRegist(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_LOG_IN == recv_header->netCmd)
	{
		//登录
		ProcessCmdLogIn(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_LOG_OUT == recv_header->netCmd)
	{
		//登出
		ProcessCmdLogOut(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_MESSAGE == recv_header->netCmd)
	{
		//消息
		ProcessCmdMessage(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_GETALL_INFO == recv_header->netCmd)
	{
		//获取所有客户端信息
		ProcessCmdGetAllInfo(sockfd, recv_buf, recv_len, cliaddr);
	}
	else if(CMD_UNREGISTER == recv_header->netCmd)
	{
		//注销
		ProcessCmdUnRegist(sockfd, recv_buf, recv_len, cliaddr);
	}
	return OK;
}

void *QserverTask(void *arg) 
{ 
	struct sockaddr_in servaddr, cliaddr; 
	socklen_t cliaddr_len; 
	int sockfd; 
	char recv_buf[MAXLINE];
	int recv_len;

	RamStorInitInfo();

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(SERV_PORT); 

	bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 

	printf("[server]Accepting connections ...\n"); 
	while (1) 
	{ 
		cliaddr_len = sizeof(cliaddr);
		bzero(recv_buf, sizeof(recv_buf));
		recv_len = recvfrom(sockfd, recv_buf, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len); 
		if (recv_len == -1)
		{
			printf("[server]recvfrom error!\n"); 
		}
        else
		{
			ProcessClientRequest(sockfd, recv_buf, recv_len, &cliaddr);
		}
	} 
} 

void *SerialCmdTask(void *arg) 
{
	char buf[128] = {0};
	CLIENT_INFO_NODE *pNode = NULL;
	int i = 0;
	
	while(1)
	{
		scanf("%s", buf);
		if(0 == strcmp("getClient", buf))
		{
			int nodePoolUsed = 0;
			for(i = 0; i < MAX_CLIENT_NUM; i++)
			{
				if(g_clientInfoNodePool[i].used)
				{
					nodePoolUsed++;
				}
			}
			printf("[server]total client:%d, NodePool:%d/%d\n", RamStorListCount(), nodePoolUsed, MAX_CLIENT_NUM);
			i = 0;
			for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
			{
				printf("[%d]clientNo:%d, clientName:%s, ip:%s, login:%d\n", i+1, pNode->client.clientNo, pNode->client.clientName, 
					inet_ntoa(pNode->client.cliaddr.sin_addr), pNode->client.login);
				i++;
			}
		}
		usleep(200000);
	}
}

int main(int argc, char **argv)
{
	pthread_t ntid;
	pthread_t ntid2;
	
	if (0 != pthread_create(&ntid, NULL, QserverTask, NULL))
	{
		printf("create QserverTask fail!\n");
	}
	if (0 != pthread_create(&ntid2, NULL, SerialCmdTask, NULL))
	{
		printf("create QserverTask fail!\n");
	}
	while(1)
	{
		pause();
	}
}

