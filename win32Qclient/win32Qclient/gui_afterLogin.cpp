#include "stdafx.h"
#include "win32Qclient.h"
#include "comsubdefine.h"
#include "q_client.h"
#include "gui_afterLogin.h"
#include "mmsystem.h"//��������ͷ�ļ�
#pragma comment(lib,"winmm.lib")//��������ͷ�ļ���

//����ListView�ȹ����ؼ���
//#pragma comment( lib, "comctl32.lib" )

#define CHAT_CLASS_NAME       "chat"
#define AFTER_LOGIN_CLASS_NAME "afterLgin"
#define MY_TIMEER_AFTER_LOGIN   2
#define UI_TALK_DATA_LENG  200*1024 //UI����������ʾ���� 200kB

BOOL g_ClientChatFlag[MAX_CLIENT_NUM] = {0};

HINSTANCE hInst_;
char szPointLogin[64] = {0};
char chat_name[32] = {0};
char *g_talkData[64] = {0};

WNDPROC OldScrollProc;


LRESULT CALLBACK ScrollProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int iVScrollBarPos = 0;
	static int cxClient = 0;
	static int cyClient = 0;
	static SCROLLINFO si; //�䳤��������Ϣ�ṹ��
	int iVertPos = 0;
	switch (message)
	{
		case WM_CREATE:
			break;

		case WM_TIMER:
			break;

		case WM_COMMAND:
			break;

		case WM_PAINT:
			//lff:�������ʹ�ÿؼ�ԭ�����������������ƿؼ�����
			return OldScrollProc(hWnd, message, wParam, lParam);
			
		case WM_VSCROLL: //��ֱ��������Ϣ
			si.cbSize	= sizeof(si);
			si.fMask	= SIF_ALL;
			GetScrollInfo(hWnd, SB_VERT, &si);
	 
			iVertPos = si.nPos;
			switch(LOWORD(wParam))
			{
			case SB_TOP: //�ö�(�Ȱ���Shift�����ţ�Ȼ���������������ϲ���������ö�)
               si.nPos = si.nMin;
               break;
               
          	case SB_BOTTOM://�õ�(ͬ�ö�)
               si.nPos = si.nMax;
               break;
			
			case SB_LINEUP:
				//ÿ�ι���ͼƬ�仯10������
				si.nPos -= 10;
				break;
			
			case SB_LINEDOWN:
				si.nPos += 10;
				break;
			
			case SB_PAGEUP:
				//ÿ�η�ҳ������һ�����ͻ����Ĵ�С
				//iVScrollBarPos -= cyClient;
				break;
				
			case SB_PAGEDOWN:
				//iVScrollBarPos += cyClient;
				break;
				
			case SB_THUMBTRACK:
				si.nPos = HIWORD (wParam) ;
				break;
				
			default :
				break;
			}
			si.fMask	= SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
			GetScrollInfo(hWnd, SB_VERT, &si);
			
			if(iVertPos != si.nPos)
			{
				//����InvalidateRect��ScrollWindow��Ч����ͬ
				InvalidateRect(hWnd, NULL, FALSE);
				//ScrollWindow(hwnd, 0, iVertPos - si.nPos, NULL, NULL);
			}
			break;
			
		case WM_KEYDOWN:
			break;

		case WM_CHAR: 
			break;

		case WM_LBUTTONDOWN:
			break;

		case WM_RBUTTONDOWN: 
			break;

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			//PostQuitMessage(0);
			break;
			
		default:
			break;
	}
	//lff:�������ʹ�ô���Ĭ�ϴ�����DefWindowProc������ؼ���Ϣ����ʹ��������Ч
	return OldScrollProc(hWnd, message, wParam, lParam); // DefWindowProc
}


void ui_chat_handle_create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	char cap[32] = {0};
	HWND hwndScroll;
	
	GetWindowText(hWnd, cap, 32);
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		if(!strncmp(g_allClientInfo.client[i].clientName, cap, 32))
			break;
	}
	if(i < MAX_CLIENT_NUM)
	{
		if(!g_talkData[i])
		{
			g_talkData[i] = (char*)malloc(UI_TALK_DATA_LENG); //200kB ���ڱ�����������
			if(g_talkData[i])
			{
				memset(g_talkData[i], 0, UI_TALK_DATA_LENG);
			}
		}
		set_ui_Qclient_chat_hwnd(hWnd, i, (int)g_allClientInfo.client[i].clientNo, g_talkData[i]);
	}

	//������ʾ���ڱ༭�� ֻ��|����|��ֱ����|������
	hwndScroll = CreateWindowEx(0, "edit", "�����¼", WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL, //WS_HSCROLL
		0, 0, 485, 250, hWnd, (HMENU)ID_USR_CHAT_OUTPUT_STATIC, hInst_, NULL);
	if((i < MAX_CLIENT_NUM) && (0 < strlen(g_talkData[i])))
	{
		SetWindowText(hwndScroll, g_talkData[i]);
	}
	SendDlgItemMessage(hWnd, ID_USR_CHAT_OUTPUT_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE);
	//SetScrollRange(hwndScroll, SB_VERT, 0, 250, FALSE);
	//�����������Ϣ��Ҫ�޸Ŀؼ�������
	//OldScrollProc = (WNDPROC) SetWindowLong (hwndScroll, GWL_WNDPROC, (LONG) ScrollProc) ;

	//�������봰�ڱ༭�� ����|��ֱ����
	CreateWindowEx(0, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL, //���� ��ֱ�������
		0, 252, 500, 80, hWnd, (HMENU)ID_USR_CHAT_INPUT_EDIT, hInst_, NULL);
	SendDlgItemMessage(hWnd, ID_USR_CHAT_INPUT_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "BUTTON", "�ر�", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		335, 340, 60, 25, hWnd, (HMENU)ID_USR_CHAT_CLOSE_BUTTON, hInst_, NULL);
	SendDlgItemMessage(hWnd, ID_USR_CHAT_CLOSE_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "BUTTON", "����", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		410, 340, 60, 25, hWnd, (HMENU)ID_USR_CHAT_SEND_BUTTON, hInst_, NULL);
	SendDlgItemMessage(hWnd, ID_USR_CHAT_SEND_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE);
}

void ui_chat_handle_timer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

}

int ui_chat_handle_command(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	char send_data[1024] = {0};
	int i = 0;
	char cap[32] = {0};
	
	wmId	= LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	
	switch (wmId)
	{
		case ID_USR_CHAT_SEND_BUTTON:
			memset(send_data, 0, sizeof(send_data));
			SendDlgItemMessage(hWnd, ID_USR_CHAT_INPUT_EDIT, WM_GETTEXT, (WPARAM)sizeof(send_data), (LPARAM)send_data);
			if(0 == strlen(send_data))
			{
				MessageBox(hWnd, "���Ͳ���Ϊ��!", "error", MB_OK);
				break;
			}
			GetWindowText(hWnd, cap, 32);
			for(i = 0; i < g_allClientInfo.clientNum; i++)
			{
				if(!strncmp(g_allClientInfo.client[i].clientName, cap, 32))
					break;
			}
			if(i < MAX_CLIENT_NUM)
			{
				ClientSendMessage(sockfd, &servaddr, send_data, g_allClientInfo.client[i].clientNo);
			}
			SendMessage(hWnd, WM_USER_RECV_DATA, (WPARAM)(send_data), 0);
			SetWindowText(GetDlgItem(hWnd, ID_USR_CHAT_INPUT_EDIT), NULL);
			break;

		case ID_USR_CHAT_CLOSE_BUTTON:
			SendMessage(hWnd,WM_CLOSE,0,0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


void ui_chat_handle_paint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HPEN pen;
	HPEN oldpen;
	char *talkdata = NULL;
	RECT rect;
	
	hdc = BeginPaint(hWnd, &ps);

	pen = (HPEN)CreatePen(PS_SOLID, 1, RGB(192,192,192));
	oldpen = (HPEN)SelectObject(hdc, pen);
	MoveToEx(hdc, 0, 250, NULL);
	LineTo(hdc, 500, 250);
	SelectObject(hdc, oldpen);
	DeleteObject(pen);

	#if 0
	talkdata = get_ui_talkData_by_hwnd(hWnd);
	//GetDlgItem(hWnd, ID_USR_CHAT_OUTPUT_STATIC)
	GetClientRect(hWnd, &rect);
	char text[128] = {0};
	sprintf(text, "%d %d %d %d", rect.left, rect.top, rect.right, rect.bottom);
	//SetWindowText(hWnd, text);
	DrawText(hdc, talkdata, -1, &rect, DT_CALCRECT);
	TextOut(hdc, 20, 20, talkdata, strlen(talkdata));
	#endif
	
	EndPaint(hWnd, &ps);
	
	return ;

}

void ui_chat_handle_size(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cxClient = 0;
	int cyClient = 0;
	char text[128] = {0};
	
	//���ڴ�С�ı�ʱ�յ�����Ϣ��F12�ɲ鿴��Ϣ������˼��msdn�鵽��ȡ�´��ڿ�͸ߵķ���
	cxClient = LOWORD(wParam);
	cyClient = HIWORD(lParam);
	sprintf(text, "x:%d, y:%d", cxClient, cyClient);
	SetWindowText(hWnd, text);

	#if 0
	//�趨�������ķ�Χ
	SetScrollRange(hwnd, SB_VERT, 0, 600 - cyClient, FALSE);
	//��ȡ����������λ�ã����жϴ��ڴ�С�ı��Ժ�������Ƿ񳬳���Ӧ�е����Χ
	iHScrollBarPos = min(cxBitmap - cxClient, max(0, iHScrollBarPos));
	iVScrollBarPos = min(cyBitmap - cyClient, max(0, iHScrollBarPos));
	//������������������Χ���������������Χ��ˢ�´���
	if(iHScrollBarPos != GetScrollPos(hwnd, SB_HORZ))
	{
		SetScrollPos(hwnd, SB_HORZ, iHScrollBarPos, TRUE);
		InvalidateRect(hwnd, NULL, FALSE);
	}
	if(iVScrollBarPos != GetScrollPos(hwnd, SB_VERT))
	{
		SetScrollPos(hwnd, SB_VERT, iVScrollBarPos, SB_VERT);
		InvalidateRect(hwnd, NULL, FALSE);
	}
	#endif
}

void ui_chat_handle_usr_recv_data(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int recvflag = (int)lParam;
	char *data = get_ui_talkData_by_hwnd(hWnd);
	char cap[32] = {0};

	if(!data)
		return;
	
	GetWindowText(hWnd, cap, 32);
	if((UI_TALK_DATA_LENG-1024) < strlen(data))
	{
		//�����������ݳ���199kBʱ,���ԭ������
		memset(data, 0, UI_TALK_DATA_LENG);
	}
	if(recvflag) //�յ�������
	{
		PlaySound("msg.wav", NULL, SND_FILENAME|SND_ASYNC);
		sprintf(data+strlen(data), "%s:\r\n", cap);
	}
	else //�Լ����͵�����
	{
		strcpy(data+strlen(data), "��:\r\n");
	}
	strcpy(data+strlen(data), (char*)wParam);
	strcpy(data+strlen(data), "\r\n");
	SetWindowText(GetDlgItem(hWnd, ID_USR_CHAT_OUTPUT_STATIC), data);
	//InvalidateRect(GetDlgItem(hWnd, ID_USR_CHAT_OUTPUT_STATIC), NULL, false); //�ػ���������
	SendMessage(GetDlgItem(hWnd, ID_USR_CHAT_OUTPUT_STATIC), WM_VSCROLL, SB_BOTTOM, 0); //�յ���Ϣʱ�ù������õ�
}

void ui_chat_handle_keydown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

}

void ui_chat_handle_char(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
}

void ui_chat_handle_lbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	#if 0
	char szPoint[64] = {0};
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	wsprintf(szPoint, "X=%d, Y=%d", x, y);
	SetWindowText(hWnd, szPoint);
	#endif
}

void ui_chat_handle_close(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	char cap[32] = {0};

	GetWindowText(hWnd, cap, 32);
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		if(!strncmp(g_allClientInfo.client[i].clientName, cap, 32))
			break;
	}
	if(i < MAX_CLIENT_NUM)
	{
		if(g_ClientChatFlag[i])
		{
			g_ClientChatFlag[i] = 0;
		}
		set_ui_Qclient_chat_hwnd((HWND)INVALID_HWND, i, -1, NULL);
	}
	DestroyWindow(hWnd);
}

LRESULT CALLBACK ui_chatWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int iVScrollBarPos = 0;
	switch (message)
	{
		case WM_CREATE:
			ui_chat_handle_create(hWnd, message, wParam, lParam);
			break;

		case WM_TIMER:
			ui_chat_handle_timer(hWnd, message, wParam, lParam);
			break;

		case WM_COMMAND:
			ui_chat_handle_command(hWnd, message, wParam, lParam);
			break;

		case WM_PAINT:
			ui_chat_handle_paint(hWnd, message, wParam, lParam);
			break;

		#if 0
		case WM_SIZE:
			//���ڴ�С�ı�ʱ�յ�����Ϣ��F12�ɲ鿴��Ϣ������˼��msdn�鵽��ȡ�´��ڿ�͸ߵķ���
			ui_chat_handle_size(hWnd, message, wParam, lParam);
			break;
		#endif

		case WM_USER_RECV_DATA:
			ui_chat_handle_usr_recv_data(hWnd, message, wParam, lParam);
			break;

		#if 0
		case WM_VSCROLL:
		MessageBox(hWnd, "WM_VSCROLL", "test..", MB_OK);
		//��ֱ��������Ϣ��F12�鿴��Ϣ���Դ�MSDN��ȡ��Ϣ��ϸ����
		switch(LOWORD(wParam))
		{
		case SB_LINEUP:
			//ÿ�ι���ͼƬ�仯10������
			iVScrollBarPos -= 10;
			break;
		case SB_LINEDOWN:
			iVScrollBarPos += 10;
			break;
		case SB_PAGEUP:
			//ÿ�η�ҳ������һ�����ͻ����Ĵ�С
			//iVScrollBarPos -= cyClient;
			break;
		case SB_PAGEDOWN:
			//iVScrollBarPos += cyClient;
			break;
		case SB_THUMBTRACK:
			//iVScrollBarPos = HIWORD(wparam);
			break;
		default :
			break;
		}
		//�жϹ�����Ĺ������Ƿ񳬹����ֵ����Сֵ������������ֵ������Сֵ����ȡ���ֵ��0��������ڵ�ǰֵ
		//iVScrollBarPos = min(cyBitmap - cyClient, max(0, iVScrollBarPos));
		//���������λ�÷����仯�������ù�����λ�ú�ˢ����Ļ
		if(iVScrollBarPos != GetScrollPos(hWnd, SB_VERT))
		{
			SetScrollPos(hWnd, SB_VERT, iVScrollBarPos, TRUE);
			//����������ΪFALSE���Դ���ȼ�����Ļ��˸�����Գ���һ�¡�
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
		#endif
			
		case WM_KEYDOWN:
			ui_chat_handle_keydown(hWnd, message, wParam, lParam);
			break;

		case WM_CHAR: //WM_KENDOWN��Ϣ��TranslateMessageת��ΪWM_CHAR��Ϣ
			ui_chat_handle_char(hWnd, message, wParam, lParam);
			break;

		case WM_LBUTTONDOWN: //������������Ϣ
			ui_chat_handle_lbuttondown(hWnd, message, wParam, lParam);
			break;

		case WM_RBUTTONDOWN: 
			break;

		case WM_CLOSE:
			ui_chat_handle_close(hWnd, message, wParam, lParam);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//����������洰��,�ô��ڹر�ʱ���ᵼ��WinMain�����˳�(��Ϊ�ǲ�ͬ�߳�)
int ui_chat_window_start(HINSTANCE hInst, HWND hParentWnd, char *caption)
{
	HWND hWnd;
	WNDCLASSEX wcex;
	MSG msg;
	HACCEL hAccelTable;

	hInst_ = hInst;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= ui_chatWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= CHAT_CLASS_NAME;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	RegisterClassEx(&wcex);
	hWnd = CreateWindow(CHAT_CLASS_NAME, caption, WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX&~WS_SIZEBOX,
		CW_USEDEFAULT, 0, 500, 415, hParentWnd, NULL, hInst, NULL);
	if (!hWnd)
	{
	  return FALSE;
	}
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_WIN32QCLIENT));

	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return TRUE;
}

DWORD WINAPI QchatTask(LPVOID p)
{
	char caption[32] = {0};

	sprintf(caption, "%s", (char*)p);
	ui_chat_window_start(hInst_, NULL, caption);
	
	return 0;
}


//��listview�ؼ������
void InsertColumn(HWND hList)
{
    LV_COLUMN lvc;

    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.iSubItem = 0;
	lvc.fmt = LVCFMT_CENTER;
    lvc.pszText = "�˺�";
    lvc.cx = 170;
    SendMessage(hList, LVM_INSERTCOLUMN, 0, (long)&lvc);

	lvc.iSubItem = 1;
    lvc.pszText = "״̬";
    lvc.cx = 130;
    SendMessage(hList, LVM_INSERTCOLUMN, 1, (long)&lvc);
}

//��listview�ؼ������
void InsertItem(HWND hList,char *text,int x,int y)
{  
	LVITEM item;  

	item.mask=LVIF_TEXT;  

	item.pszText=text;  

	item.iItem=x;  

	item.iSubItem=y;

	if(y==0)   
		SendMessage(hList, LVM_INSERTITEM, 0, LPARAM(&item)); 
	else 
		SendMessage(hList, LVM_SETITEM, 0, LPARAM(&item));
}

//��listview�ؼ�ɾ����
void DeleteItem(HWND hList, int idx)
{  
	SendMessage(hList, LVM_DELETEITEM, (WPARAM)idx, 0);
}

//ɾ��listview�ؼ���������
void DeleteAllItem(HWND hList)
{
	SendMessage(hList, LVM_DELETEALLITEMS, 0, 0);
}

int ui_afterLogin_handle_create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hList;
	int i = 0;
	
	set_ui_Qclient_afterLogin_hwnd(hWnd);
	//SetWindowText(hWnd, "QQ 2009 Login");

	//InitCommonControls();
	hList = CreateWindowEx(NULL, "SysListView32", NULL, LVS_REPORT | WS_CHILD | WS_VISIBLE, 0, 0, 300, 550, hWnd, (HMENU)ID_USR_AFTERLOGIN_PANEL, hInst_, NULL); //WC_LISTVIEW
	InsertColumn(hList);
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		InsertItem(hList, g_allClientInfo.client[i].clientName, i, 0);
		if(g_allClientInfo.client[i].login)
		{
			InsertItem(hList, "����", i, 1);
		}
		else
		{
			InsertItem(hList, "����", i, 1);
		}
	}

	#if 1
	SendMessage(hList, LVM_SETTEXTCOLOR, 0, RGB(0, 0, 0));
	SendMessage(hList, LVM_SETBKCOLOR, 0, RGB(215, 235, 255));
	SendMessage(hList, LVM_SETTEXTBKCOLOR, 0, RGB(138, 197, 255));
	#endif
	
	return 0;
}

int ui_afterLogin_handle_timer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ui_afterLogin_handle_cmd_passwd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	return 0;
}


int ui_afterLogin_handle_command(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	FILE *file = NULL;
	
	wmId	= LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	// �����˵�ѡ��:
	switch (wmId)
	{
		case 0:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int ui_afterLogin_handle_notify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD  threadId;
	NM_LISTVIEW* pnmv = (NM_LISTVIEW FAR *) lParam;

	if(pnmv->hdr.code == LVN_COLUMNCLICK)//�������, ��ͷ
	{
		//MessageBox(hWnd, "1!", "1", MB_OK);
	}
	else if(pnmv->hdr.code == LVN_ITEMACTIVATE)//�����б����ĳһ�Ҳ�����У�˫��������
	{
		//MessageBox(hWnd, "2!", "2", MB_OK);
	}
	else if(pnmv->hdr.code == NM_CLICK)//�����б����ĳһ��
	{
		int i = 0;
		
		//MessageBox(hWnd, str, "3", MB_OK);
		memset(chat_name, 0, sizeof(chat_name));
		ListView_GetItemText(pnmv->hdr.hwndFrom, pnmv->iItem, pnmv->iSubItem, chat_name, 32);
		
		for(i = 0; i < g_allClientInfo.clientNum; i++)
		{
			if(!strncmp(g_allClientInfo.client[i].clientName, chat_name, 32))
				break;
		}
		if(i < MAX_CLIENT_NUM)
		{
			if(0 == g_ClientChatFlag[i])
			{
				g_ClientChatFlag[i] = 1;
				//lff:���������������(�����Ĵ��ڴ�����Ϣ,�������߳�)
				CreateThread(NULL, 0, QchatTask, chat_name, 0, NULL); 
			}
		}
	}
	return 0;
}

int ui_afterLogin_handle_paint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	
	hdc = BeginPaint(hWnd, &ps); //lff:��ȡ���ڿͻ�����Ч������豸�������(������Ļ��һ������)
	
	EndPaint(hWnd, &ps);
	
	return 0;
}

int ui_afterLogin_handle_keydown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{  
        case VK_LEFT://�����  
            break;  
			
        case VK_RIGHT:  
            break; 
			
        case VK_UP:
            break; 
			
        case VK_DOWN:
            break; 

		case VK_SPACE: //�ո��
			break;
			
        default:  
            break;
	}
	return 0;
}

void ui_afterLogin_handle_changed(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	HWND hList = GetDlgItem(hWnd, ID_USR_AFTERLOGIN_PANEL);
	
	DeleteAllItem(hList);
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		InsertItem(hList, g_allClientInfo.client[i].clientName, i, 0);
		if(g_allClientInfo.client[i].login)
		{
			InsertItem(hList, "����", i, 1);
		}
		else
		{
			InsertItem(hList, "����", i, 1);
		}
	}
	//UpdateWindow(hWnd);
}

void ui_afterLogin_handle_usr_recv_data(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

	memset(chat_name, 0, sizeof(chat_name));
	memcpy(chat_name, (char*)wParam, sizeof(chat_name));
	for(i = 0; i < g_allClientInfo.clientNum; i++)
	{
		if(!strncmp(g_allClientInfo.client[i].clientName, chat_name, 32))
			break;
	}
	if(i < MAX_CLIENT_NUM)
	{
		if(0 == g_ClientChatFlag[i])
		{
			g_ClientChatFlag[i] = 1;
			//lff:���������������(�����Ĵ��ڴ�����Ϣ,�������߳�)
			CreateThread(NULL, 0, QchatTask, chat_name, 0, NULL); 
		}
	}
}

int ui_afterLogin_handle_char(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	return 0;
}

int ui_afterLogin_handle_lbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	wsprintf(szPointLogin, "QQ (X=%d, Y=%d)", x, y);
	SetWindowText(hWnd, szPointLogin);
	return 0;
}


int ui_afterLogin_handle_rbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ui_afterLogin_handle_close(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	set_ui_Qclient_afterLogin_hwnd((HWND)INVALID_HWND);

	ClientSendLogOut(sockfd, &servaddr);
	
	DestroyWindow(hWnd);
	
	return 0;
}


//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK ui_afterLoginWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			ui_afterLogin_handle_create(hWnd, message, wParam, lParam);
			break;

		case WM_TIMER:
			ui_afterLogin_handle_timer(hWnd, message, wParam, lParam);
			break;

		case WM_COMMAND:
			ui_afterLogin_handle_command(hWnd, message, wParam, lParam);
			break;

		case WM_NOTIFY:
			ui_afterLogin_handle_notify(hWnd, message, wParam, lParam);
			break;

		case WM_PAINT:
			ui_afterLogin_handle_paint(hWnd, message, wParam, lParam);
			break;

		case WM_KEYDOWN:
			ui_afterLogin_handle_keydown(hWnd, message, wParam, lParam);
			break;

		case WM_USER_CHANGED:
			ui_afterLogin_handle_changed(hWnd, message, wParam, lParam);
			break;

		case WM_USER_RECV_DATA:
			ui_afterLogin_handle_usr_recv_data(hWnd, message, wParam, lParam);
			break;
			
		case WM_CHAR: //lff: WM_KENDOWN��Ϣ��TranslateMessageת��ΪWM_CHAR��Ϣ
			ui_afterLogin_handle_char(hWnd, message, wParam, lParam);
			break;

		case WM_LBUTTONDOWN: //lff:������������Ϣ
			ui_afterLogin_handle_lbuttondown(hWnd, message, wParam, lParam);
			break;

		case WM_RBUTTONDOWN: //lff:����Ҽ�����
			ui_afterLogin_handle_rbuttondown(hWnd, message, wParam, lParam);
			break;

		case WM_CLOSE:
			ui_afterLogin_handle_close(hWnd, message, wParam, lParam);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
ATOM MyRegisterClass_AfterLogin(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= ui_afterLoginWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= AFTER_LOGIN_CLASS_NAME;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

int ui_afterLogin_window_start(HINSTANCE hInst, HWND hParentWnd, char *cap)
{
	HWND hWnd;

	hInst_ = hInst;
	
	MyRegisterClass_AfterLogin(hInst);
	hWnd = CreateWindow(AFTER_LOGIN_CLASS_NAME, cap, WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX&~WS_SIZEBOX,
		CW_USEDEFAULT, 0, 300, 600, hParentWnd, NULL, hInst, NULL);
	if (!hWnd)
	{
	  return FALSE;
	}
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	return TRUE;
}

