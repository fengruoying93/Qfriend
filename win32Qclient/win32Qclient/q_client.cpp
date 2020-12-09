#include "stdafx.h"
#include <errno.h>
#include "q_client.h"
#include "comsubdefine.h"
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

char ServerIP[16] = "127.0.0.1";
UINT32 ServerPort = 6666;

#define MY_CLIENT_NAME  "client1"

UINT32 g_client_no;

ALL_CLIENT_INFO g_allClientInfo;

#ifndef WIN32
int sockfd;
#else
SOCKET sockfd;
#endif
struct sockaddr_in servaddr;

int ClientSendRegist(int sockfd, struct sockaddr_in *servaddr)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_REGISTER;
	message.header.clientNo = 0;
	memcpy(message.header.clientName, g_account, NAME_LEN);
	memcpy(message.send_data, g_account, sizeof(g_account));
	memcpy(message.send_data+NAME_LEN, g_passwd, sizeof(g_passwd));
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
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
	memcpy(message.header.clientName, g_account, NAME_LEN);
	memcpy(message.send_data, g_account, sizeof(g_account));
	memcpy(message.send_data+NAME_LEN, g_passwd, sizeof(g_passwd));
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
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
	memcpy(message.header.clientName, g_account, NAME_LEN);
	memcpy(message.send_data, g_account, sizeof(g_account));
	memcpy(message.send_data+NAME_LEN, g_passwd, sizeof(g_passwd));
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
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
	memcpy(message.header.clientName, g_account, NAME_LEN);
	memcpy(message.send_data, g_account, sizeof(g_account));
	memcpy(message.send_data+NAME_LEN, g_passwd, sizeof(g_passwd));
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
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
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientRegist sendto error!\n");
		return ERROR;
	}
	printf("[client]send getAllInfo ok\n");
	return OK;
}

int ClientSendMessage(int sockfd, struct sockaddr_in *servaddr, char *data, unsigned int dst_no)
{
	SEND_MESSAGE message = {0};

	message.header.netCmd = CMD_MESSAGE;
	message.header.clientNo = g_client_no;
	message.header.destNo = dst_no;
	memcpy(message.header.clientName, g_account, NAME_LEN);
	memcpy(message.send_data, data, strlen(data));
	if(-1 == sendto(sockfd, (char*)&message, sizeof(message), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)))
	{
		printf("[client]ClientSendMessage sendto error!\n");
	}
	else
	{
		printf("[client]ClientSendMessage sendto success:%s\n", message.send_data);
	}
	return 0;
}

#if 0
void *ClientSendTask(void *arg)
{
	int sockfd = *(int *)arg;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr)); 
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
#endif

int ProcessCmdGetAllInfo(int sockfd, char *recv_buf, int recv_len, struct sockaddr_in *cliaddr)
{
	int count = 0;
	CLIENT_INFO *result = NULL;
	ALL_CLIENT_INFO *clientInfo = NULL;
	int i =0;

	clientInfo = (ALL_CLIENT_INFO*)(recv_buf + sizeof(NET_CMD_HEADER));
	memset(&g_allClientInfo, 0, sizeof(g_allClientInfo));
	memcpy(&g_allClientInfo, clientInfo, sizeof(ALL_CLIENT_INFO)); //lff:此处崩溃
	printf("[client]get all clientInfo:\n");
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		printf("[%d]clientNo:%d, clientName:%s, ip:%s, login:%d\n", i+1, g_allClientInfo.client[i].clientNo, g_allClientInfo.client[i].clientName, 
			inet_ntoa(g_allClientInfo.client[i].cliaddr.sin_addr), g_allClientInfo.client[i].login);
		if(0 == strcmp(g_account, g_allClientInfo.client[i].clientName))
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
	if((HWND)INVALID_HWND == get_ui_Qclient_chat_hwnd(recv_header->clientNo))
	{
		//收到数据时聊天窗口还未打开,发送UI消息启动线程创建聊天窗口
		SendMessage(get_ui_Qclient_afterLogin_hwnd(), WM_USER_RECV_DATA, (WPARAM)(recv_header->clientName), 0);
		Sleep(1000); //延时1s确保窗口已经创建
	}
	//给聊天窗口发送数据
	SendMessage(get_ui_Qclient_chat_hwnd(recv_header->clientNo), WM_USER_RECV_DATA, (WPARAM)(recv_buf+sizeof(NET_CMD_HEADER)), 1); 
	
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
		if (OK == ProcessCmdGetAllInfo(sockfd, recv_buf, recv_len, cliaddr))
		{
			if((HWND)INVALID_HWND != get_ui_Qclient_afterLogin_hwnd())
			{
				PostMessage(get_ui_Qclient_afterLogin_hwnd(), WM_USER_CHANGED, 0, 0);
			}
			else
			{
				PostMessage(get_ui_Qclient_hwnd(), WM_USER_CHANGED, 0, 0); //lff:向UI发送登录成功消息
			}
		}
	}
	else if(CMD_MESSAGE == recv_header->netCmd)
	{
		//消息
		ProcessCmdMessage(sockfd, recv_buf, recv_len, cliaddr);
	}

	return OK;
}

#ifdef WIN32
DWORD WINAPI QclientTask(LPVOID p)
#else
void *QclientTask(void *arg)
#endif
{ 
	struct sockaddr_in cliaddr; 
	socklen_t cliaddr_len;  
	char recv_buf[MAXLINE];
	int recv_len;
	#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	#endif
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	inet_pton(AF_INET, ServerIP, &servaddr.sin_addr);
	servaddr.sin_port = htons(ServerPort); 
 
	ClientSendRegist(sockfd, &servaddr);
	ClientSendLogIn(sockfd, &servaddr);
	//ClientSendGetAllInfo(sockfd, &servaddr); //客户端登录成功后服务端自动返回所有客户端信息
	//pthread_create(&ntid, NULL, ClientSendTask, &sockfd);
	
	while (1) 
	{ 
		cliaddr_len = sizeof(cliaddr); 
		memset(recv_buf, 0, sizeof(recv_buf));
		recv_len = recvfrom(sockfd, recv_buf, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
#ifdef WIN32
		if(0) //(recv_len == -1)
#else
		if (recv_len == -1)
#endif
		{
			printf("[client]recvfrom error!\n"); 
		}
        else
		{
			ClientProcessMessage(sockfd, recv_buf, recv_len, &cliaddr);
		}
	} 
} 

