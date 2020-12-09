#ifndef Q_CLIENT_H
#define Q_CLIENT_H

#ifdef WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#define SAFE_FREE(p)  do{if(p){free(p);p = NULL;}}while(0)

#ifndef OK
#define OK      0
#endif
#ifndef ERROR
#define ERROR  -1
#endif

#define TRUE    1
#define FALSE   0

#define NAME_LEN  32
#define PWD_LEN   32
#define MESSAGE_LEN 1024
#define MAX_CLIENT_NUM 64

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;
typedef signed long long INT64;

typedef struct client_info
{
	UINT32 clientNo;
	char clientName[NAME_LEN];
	char clientPwd[PWD_LEN];
	int login; //是否登录
	struct sockaddr_in cliaddr;
}CLIENT_INFO;

typedef struct all_client_info
{
	UINT32 clientNum;
	CLIENT_INFO client[MAX_CLIENT_NUM];
}ALL_CLIENT_INFO;

typedef enum
{
	CMD_REGISTER = 0,
	CMD_LOG_IN = 1,
	CMD_LOG_OUT = 2,
	CMD_MESSAGE = 3,
	CMD_GETALL_INFO = 4,
	CMD_UNREGISTER = 5,
}NET_CMD_TYPE;

typedef struct net_cmd_header
{
	UINT32 data_len; //数据段长度
	UINT32 netCmd; //命令码, 见NET_CMD_TYPE
	UINT32 clientNo;
	UINT32 destNo;
	char clientName[NAME_LEN];
}NET_CMD_HEADER;

typedef struct send_message
{
	NET_CMD_HEADER header;
	char send_data[MESSAGE_LEN];
}SEND_MESSAGE;

extern char ServerIP[16];
extern UINT32 ServerPort;
extern char g_account[32];
extern char g_passwd[32];
extern ALL_CLIENT_INFO g_allClientInfo;
extern struct sockaddr_in servaddr;

#ifndef WIN32
extern int sockfd;
#else
extern SOCKET sockfd;
#endif

#ifdef WIN32
DWORD WINAPI QclientTask(LPVOID p);
#else
void *QclientTask(void *arg);
#endif
int ClientSendMessage(int sockfd, struct sockaddr_in *servaddr, char *data, unsigned int dst_no);
int ClientSendLogOut(int sockfd, struct sockaddr_in *servaddr);


#endif

