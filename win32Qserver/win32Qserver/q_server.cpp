#include "stdafx.h"
#include <errno.h>
#include "q_server.h"
#include "lstLib.h"
#include "myResource.h"
#ifndef WIN32
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#define MAXLINE 10*1024 
#define snprintf(buf,len, format,...) _snprintf_s(buf, len, len-1, format, __VA_ARGS__)


UINT32 ServerPort = 6666;
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
		snprintf(g_showText, sizeof(g_showText), "malloc fail!\n");
		MyPrintf();
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
		snprintf(g_showText, sizeof(g_showText), "[server]RamStorDeleteNode param error\n");
		MyPrintf();
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
		snprintf(g_showText, sizeof(g_showText), "[server]RamStorDeleteNode find none node!\n");
		MyPrintf();
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
	
	memset(&data, 0, sizeof(data));
	
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
	
	if (-1 == sendto(sockfd, (char*)&data, sizeof(data), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in)))
	{
		snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdGetAllInfo sendto error!%d:%s\n", errno, strerror(errno));
		MyPrintf();
	}
	
	return OK;
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
			snprintf(g_showText, sizeof(g_showText), "[server]client %s is already registered!\n", client_name);
			MyPrintf();
			return ERROR;
		}
	}

	clientNode.client.clientNo = RamStorListCount()+1;
	memcpy(clientNode.client.clientName, recv_buf+sizeof(NET_CMD_HEADER), NAME_LEN);
	memcpy(clientNode.client.clientPwd, recv_buf+sizeof(NET_CMD_HEADER)+NAME_LEN, PWD_LEN);
	memcpy(&clientNode.client.cliaddr, cliaddr, sizeof(struct sockaddr_in));
	
	RamStorAddNode(&clientNode);
	
	snprintf(g_showText, sizeof(g_showText), "[server]insert regist -- clientNo:%d, clientName:%s, ip:%s, total_clientNum:%d\n", clientNode.client.clientNo, 
		clientNode.client.clientName, inet_ntoa(clientNode.client.cliaddr.sin_addr), RamStorListCount());
	MyPrintf();
	
	return OK;
}

int ProcessCmdUnRegist(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf+sizeof(NET_CMD_HEADER);
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;

	if(OK == RamStorDeleteNode(client_name))
	{
		snprintf(g_showText, sizeof(g_showText), "[server]unRegist OK -- clientNo:%d, clientName:%s, total_clientNum:%d\n", recv_header->clientNo, 
			client_name, RamStorListCount());
		MyPrintf();
	}
	
	return OK;
}


int ProcessCmdLogIn(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	char *client_name = recv_buf + sizeof(NET_CMD_HEADER);
	char *client_pwd = client_name + NAME_LEN;
	CLIENT_INFO_NODE *pNode = NULL;

	if(!recv_buf || !cliaddr)
	{
		snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdLogIn param error!\n");
		MyPrintf();
		return ERROR;
	} 
	
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(!strncmp(client_name, pNode->client.clientName, NAME_LEN))
		{
			if(strncmp(client_pwd, pNode->client.clientPwd, PWD_LEN)) //登录密码和注册密码不一致
			{
				return ERROR;
			}
			snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdLogIn: find client%s!\n", client_name);
			MyPrintf();
			pNode->client.login = 1;
			memcpy(&pNode->client.cliaddr, cliaddr, sizeof(struct sockaddr_in));
			snprintf(g_showText, sizeof(g_showText), "[server]client login success -- clientNo:%u, clientName:%s, ip:%s, total_clientNum:%u\n", pNode->client.clientNo, 
				pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr), RamStorListCount());
			MyPrintf();
		}
	}

	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(pNode->client.login)
		{
			//客户端登录成功后服务端自动发送所有客户端信息, 用于客户端显示
			ProcessCmdGetAllInfo(sockfd, NULL, 0, &pNode->client.cliaddr);
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
		snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdLogOut param error!\n");
		MyPrintf();
		return ERROR;
	} 
	
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(!strncmp(client_name, pNode->client.clientName, NAME_LEN))
		{
			snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdLogOut: find client%s!\n", client_name);
			MyPrintf();
			pNode->client.login = 0;
			snprintf(g_showText, sizeof(g_showText), "[server]client logout success -- clientNo:%d, clientName:%s, ip:%s, total_clientNum:%d\n", pNode->client.clientNo, 
				pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr), RamStorListCount());
			MyPrintf();
			memset(&pNode->client.cliaddr, 0, sizeof(struct sockaddr_in));
		}
	}

	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(pNode->client.login)
		{
			//客户端登出成功后服务端自动发送所有客户端信息, 用于客户端显示
			ProcessCmdGetAllInfo(sockfd, NULL, 0, &pNode->client.cliaddr);
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

	memset(&msg, 0, sizeof(SEND_MESSAGE));
	send_len = (recv_len < sizeof(SEND_MESSAGE))?(recv_len):(sizeof(SEND_MESSAGE));
	memcpy(&msg, recv_buf, send_len);

	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		if(recv_header->destNo == pNode->client.clientNo)
		{
			snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdMessage: find destClient%d:%s, ip:%s\n", pNode->client.clientNo, pNode->client.clientName, inet_ntoa(pNode->client.cliaddr.sin_addr));
			MyPrintf();
			if (-1 == sendto(sockfd, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&pNode->client.cliaddr, sizeof(struct sockaddr_in)))
			{
				snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdMessage sendto error!%d:%s\n", errno, strerror(errno));
				MyPrintf();
				return ERROR;
			}
			else
			{
				return OK;
			}
		}
	}
	snprintf(g_showText, sizeof(g_showText), "[server]ProcessCmdMessage: can not find dest client%d!\n", recv_header->destNo);
	MyPrintf();
	return ERROR;
}

int ProcessClientRequest(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	NET_CMD_HEADER *recv_header = (NET_CMD_HEADER *)recv_buf;

	if(!recv_buf || !cliaddr)
	{
		snprintf(g_showText, sizeof(g_showText), "[server]ProcessClientRequest param error!\n");
		MyPrintf();
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

void DisplayClientInfo(void) 
{
	char buf[128] = {0};
	CLIENT_INFO_NODE *pNode = NULL;
	int i = 0;
	
	int nodePoolUsed = 0;
	for(i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if(g_clientInfoNodePool[i].used)
		{
			nodePoolUsed++;
		}
	}
	sprintf(g_showText, "[server]total client:%d, NodePool:%d/%d\n", RamStorListCount(), nodePoolUsed, MAX_CLIENT_NUM);
	i = 0;
	for(pNode = (CLIENT_INFO_NODE*)lstFirst(&g_clientInfoList); NULL != pNode; pNode = (CLIENT_INFO_NODE*)lstNext((NODE*)pNode))
	{
		sprintf(g_showText+strlen(g_showText), "[%d]clientNo:%d, clientName:%s, clientPwd:%s, ip:%s, login:%d\n", i+1, pNode->client.clientNo, pNode->client.clientName, 
			pNode->client.clientPwd, inet_ntoa(pNode->client.cliaddr.sin_addr), pNode->client.login);
		i++;
	}
	MyPrintf();
}

#ifdef WIN32
DWORD WINAPI QserverTask(LPVOID p)
#else
void *QserverTask(void *arg)
#endif
{ 
	struct sockaddr_in servaddr, cliaddr; 
	socklen_t cliaddr_len; 
	char recv_buf[MAXLINE];
	int recv_len;
	#ifndef WIN32
	int sockfd; 
	#else
	SOCKET sockfd;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	#endif
	

	RamStorInitInfo();

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(ServerPort); 

	bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 

	snprintf(g_showText, sizeof(g_showText), "[server]Accepting connections ...\n"); 
	MyPrintf();
	while (1) 
	{ 
		cliaddr_len = sizeof(struct sockaddr_in);
		memset(recv_buf, 0, sizeof(recv_buf));
		recv_len = recvfrom(sockfd, recv_buf, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len); 
		#ifdef WIN32
		if (recv_len == -1)
		#else
		if (recv_len == -1)
		#endif
		{
			snprintf(g_showText, sizeof(g_showText), "[server]recvfrom error!\n"); 
			MyPrintf();
			int errno_ = GetLastError();
		}
        else
		{
			ProcessClientRequest(sockfd, recv_buf, recv_len, &cliaddr);
		}
	} 
} 

