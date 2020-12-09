#include "stdafx.h"
#include "comsubdefine.h"

typedef struct _client_hwnd_
{
	HWND hwnd;
	unsigned int clientNo;
	char *talk_data; //ÁÄÌìÊý¾Ý»º´æ
}client_hwnd;

static HWND g_QclientHwnd = (HWND)INVALID_HWND;
static HWND g_QclientAfterLoginHwnd = (HWND)INVALID_HWND;
static client_hwnd g_QclientChatHwnd[64];

void set_ui_Qclient_hwnd(HWND hWnd)
{
	g_QclientHwnd = hWnd;
}

HWND get_ui_Qclient_hwnd(void)
{
	return g_QclientHwnd;
}

void set_ui_Qclient_afterLogin_hwnd(HWND hWnd)
{
	g_QclientAfterLoginHwnd = hWnd;
}

HWND get_ui_Qclient_afterLogin_hwnd(void)
{
	return g_QclientAfterLoginHwnd;
}

void set_ui_Qclient_chat_hwnd(HWND hWnd, int i, int clientNo, char *data)
{
	g_QclientChatHwnd[i].hwnd = hWnd;
	g_QclientChatHwnd[i].clientNo = clientNo;
	g_QclientChatHwnd[i].talk_data = data;
}

HWND get_ui_Qclient_chat_hwnd(unsigned int clientNo)
{
	int i = 0;
	for(i = 0; i < 64; i++)
	{
		if(g_QclientChatHwnd[i].clientNo == (int)clientNo)
			break;
	}
	if(i < 64)
	{
		return g_QclientChatHwnd[i].hwnd;
	}
	else
	{
		return (HWND)INVALID_HWND;
	}
}

char *get_ui_talkData_by_hwnd(HWND hwnd)
{
	int i = 0;
	for(i = 0; i < 64; i++)
	{
		if(g_QclientChatHwnd[i].hwnd == hwnd)
			break;
	}
	if(i < 64)
	{
		return g_QclientChatHwnd[i].talk_data;
	}
	else
	{
		return NULL;
	}
}


