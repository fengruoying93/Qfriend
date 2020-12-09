#ifndef Q_SERVER_H
#define Q_SERVER_H

#include <netinet/in.h>
#include "lstLib.h"

#define SAFE_FREE(p)  do{if(p){free(p);p = NULL;}}while(0)

#define OK      0
#define ERROR  -1

#define TRUE    1
#define FALSE   0

#define NAME_LEN  64
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
	int login; //是否登录
	struct sockaddr_in cliaddr;
}CLIENT_INFO;

typedef struct client_info_node
{
	NODE node;
	CLIENT_INFO client;
}CLIENT_INFO_NODE;

typedef struct client_info_node_pool
{
	CLIENT_INFO_NODE infoNode;
	int used;
}CLIENT_INFO_NODE_POOL;

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

#endif

