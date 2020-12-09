// win32Qclient.cpp : 定义应用程序的入口点。
#include "stdafx.h"
#include "win32Qclient.h"
#include "gui_afterLogin.h"
#include "comsubdefine.h"
#include "q_client.h"

#ifndef WIN32
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

#define MAX_LOADSTRING 100
#define MY_TIMEER        1


// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HFONT hFont; //自定义字体
static HBITMAP g_bmp_bkg;
char szPoint[64] = {0}; //点击坐标
char g_account[32] = {0};
char g_passwd[32] = {0};


// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK systemSetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int length = 0;
	UNREFERENCED_PARAMETER(lParam);
	
	switch (message)
	{
	case WM_INITDIALOG:
		{
			CreateWindowEx(0, "static", "服务器IP:", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
				30, 20, 80, 20, hDlg, (HMENU)ID_USR_DLG_IP_STATIC, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_IP_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体

			CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_LEFT, 
				120, 20, 120, 20, hDlg, (HMENU)ID_USR_DLG_IP_EDIT, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_IP_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

			CreateWindowEx(0, "static", "服务器端口:", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
				30, 60, 80, 20, hDlg, (HMENU)ID_USR_DLG_PORT_STATIC, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体

			CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_LEFT, 
				120, 60, 120, 20, hDlg, (HMENU)ID_USR_DLG_PORT_EDIT, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

			SetWindowText(GetDlgItem(hDlg, ID_USR_DLG_IP_EDIT), ServerIP);
			char port[16] = {0};
			_itoa(ServerPort, port, 10);
			SetWindowText(GetDlgItem(hDlg, ID_USR_DLG_PORT_EDIT), port);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			char serverPort[16] = {0};
			char text[64] = {0};
			
			length = SendDlgItemMessage(hDlg, ID_USR_DLG_IP_EDIT, WM_GETTEXTLENGTH, 0, 0);
			if(0 >= length)
			{
				printf("account param err!\n");
				return (INT_PTR)TRUE;
			}
			memset(ServerIP, 0, sizeof(ServerIP));
			SendDlgItemMessage(hDlg, ID_USR_DLG_IP_EDIT, WM_GETTEXT, (WPARAM)sizeof(ServerIP), (LPARAM)ServerIP);
			wsprintf(text, "ServerIP:%s", ServerIP);
			SetWindowText(hDlg, text);
			length = SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_EDIT, WM_GETTEXTLENGTH, 0, 0);
			if(0 >= length)
			{
				printf("account param err!\n");
				return (INT_PTR)TRUE;
			}
			memset(serverPort, 0, sizeof(serverPort));
			SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_EDIT, WM_GETTEXT, (WPARAM)sizeof(serverPort), (LPARAM)serverPort);
			wsprintf(text, "ServerPort:%s", serverPort);
			SetWindowText(hDlg, text);
			ServerPort = atoi((const char *)serverPort);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

HFONT ui_create_font(HWND hWnd)
{
	LOGFONT logFont;
	HFONT hFont;
	HDC hdc;

	//----------------------在处理WM_CREATE期间，先画控件然后使用自定义字体  
	hdc = GetDC(hWnd);
	logFont.lfHeight = MulDiv(13, GetDeviceCaps(hdc, LOGPIXELSY), 62);//13是字号大小
	ReleaseDC(hWnd, hdc);
	logFont.lfWidth = 0;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfWeight = FW_REGULAR;
	logFont.lfItalic = 0;
	logFont.lfUnderline = 0;
	logFont.lfStrikeOut = 0;
	logFont.lfCharSet = GB2312_CHARSET;
	logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality = PROOF_QUALITY;
	logFont.lfPitchAndFamily = VARIABLE_PITCH  | FF_ROMAN;
	strcpy(logFont.lfFaceName, "微软雅黑"); 

	hFont = CreateFontIndirect(&logFont);
	return hFont;
}

void ui_set_text_color(HWND hWnd)
{
	HDC hdc;
	
	hdc = GetDC(hWnd);
	COLORREF color = SetTextColor(hdc, RGB(0,0,255));
}

int ui_handle_create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	set_ui_Qclient_hwnd(hWnd);
	SetWindowText(hWnd, "QQ 2009"); //lff:窗口栏上显示标题
	SetTimer(hWnd, MY_TIMEER, 1000, NULL); 
	hFont = ui_create_font(hWnd);  //(HFONT)GetStockObject(DEFAULT_GUI_FONT);   
	g_bmp_bkg = LoadBitmap(hInst, (LPCTSTR)IDB_BITMAP1); //从资源中装载bmp
	
	//图片静态框
	CreateWindowEx(0, "static", "静态框bmp", WS_CHILD|WS_VISIBLE|SS_SUNKEN|SS_NOTIFY, 
		0, 0, 430, 79, hWnd, (HMENU)ID_USR_BKG_BMP, hInst, NULL);
	//设置SS_BITMAP风格
	HWND hWndBmp = GetDlgItem(hWnd, ID_USR_BKG_BMP);
	LONG nStyle = GetWindowLong(hWndBmp, GWL_STYLE);
	SetWindowLong(hWndBmp, GWL_STYLE, nStyle|SS_BITMAP);
	//设置图片
	SendDlgItemMessage(hWnd, ID_USR_BKG_BMP, STM_SETIMAGE, IMAGE_BITMAP, (long)g_bmp_bkg);

	CreateWindowEx(0, "static", "账号:", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		80, 100, 50, 20, hWnd, (HMENU)ID_USR_ACCOUNT_STATIC, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_ACCOUNT_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体

	CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_LEFT, 
		130, 100, 170, 25, hWnd, (HMENU)ID_USR_ACCOUNT_EDIT, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_ACCOUNT_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "static", "密码:", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		80, 140, 50, 20, hWnd, (HMENU)ID_USR_PSW_STATIC, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_PSW_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体

	CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_LEFT|ES_PASSWORD, 
		130, 140, 170, 25, hWnd, (HMENU)ID_USR_PSW_EDIT, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_PSW_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

	// 自动复选框 系统维护
	CreateWindowEx(0, "BUTTON", "记住密码", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		130, 180, 90, 20, hWnd, (HMENU)ID_USR_CHECK_BUTTON1, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_CHECK_BUTTON1,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "BUTTON", "自动登录", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		230, 180, 90, 20, hWnd, (HMENU)ID_USR_CHECK_BUTTON2, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_CHECK_BUTTON2,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "BUTTON", "登录", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		240, 220, 90, 30, hWnd, (HMENU)ID_USR_LOGIN_BUTTON, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_LOGIN_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体
	EnableWindow(GetDlgItem(hWnd, ID_USR_LOGIN_BUTTON), FALSE);

	CreateWindowEx(0, "BUTTON", "设置", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		90, 220, 90, 30, hWnd, (HMENU)ID_USR_SYSSET_BUTTON, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_SYSSET_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体
	
	return 0;
}

int ui_handle_timer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ui_handle_cmd_account(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int length = 0;
	char text[128] = {0};
	
	length = SendDlgItemMessage(hWnd, ID_USR_ACCOUNT_EDIT, WM_GETTEXTLENGTH, 0, 0);
	if(16 < length)
	{
		printf("account param err!\n");
	}
	memset(g_account, 0, sizeof(g_account));
	SendDlgItemMessage(hWnd, ID_USR_ACCOUNT_EDIT, WM_GETTEXT, (WPARAM)sizeof(g_account), (LPARAM)g_account);
	wsprintf(text, "QQ 2009 account:%s", g_account);
	SetWindowText(hWnd, text);

	if(g_account[0] && g_passwd[0])
	{
		EnableWindow(GetDlgItem(hWnd, ID_USR_LOGIN_BUTTON), TRUE);
	}
	return 0;
}

int ui_handle_cmd_passwd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int length = 0;
	char text[128] = {0};

	length = SendDlgItemMessage(hWnd, ID_USR_PSW_EDIT, WM_GETTEXTLENGTH, 0, 0);
	if(16 < length)
	{
		printf("passwd param err!\n");
	}
	memset(g_passwd, 0, sizeof(g_passwd));
	SendDlgItemMessage(hWnd, ID_USR_PSW_EDIT, WM_GETTEXT, (WPARAM)sizeof(g_passwd), (LPARAM)g_passwd);
	wsprintf(text, "QQ 2009 pwd:%s", g_passwd);
	SetWindowText(hWnd, text);

	if(g_account[0] && g_passwd[0])
	{
		EnableWindow(GetDlgItem(hWnd, ID_USR_LOGIN_BUTTON), TRUE);
	}
	return 0;
}

int ui_handle_cmd_login(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(!g_account[0] || !g_passwd[0])
	{
		return -1;
	}
	#ifdef WIN32
	DWORD  threadId;
	if (NULL == CreateThread(NULL, 0, QclientTask, 0, 0, &threadId))
	#else 
	pthread_t ntid;
	if (0 != pthread_create(&ntid, NULL, QclientTask, NULL))
	#endif
	{
		printf("create QclientTask fail!\n");
	}
	return 0;
}

int ui_handle_usr_changed(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ui_afterLogin_window_start(hInst, NULL, g_account);
	ShowWindow(hWnd, SW_HIDE);
	//SendMessage(hWnd,WM_CLOSE,0,0);
	return 0;
}


int ui_handle_command(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	FILE *file = NULL;
	
	wmId	= LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	// 分析菜单选择:
	switch (wmId)
	{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); //创建对话框
			break;

		case IDM_EXIT:
			SendMessage(hWnd,WM_CLOSE,0,0); 
			break;

		case ID_USR_ACCOUNT_EDIT:
			ui_handle_cmd_account(hWnd, message, wParam, lParam);
			break;

		case ID_USR_PSW_EDIT:
			ui_handle_cmd_passwd(hWnd, message, wParam, lParam);
			break;

		case ID_USR_LOGIN_BUTTON:
			ui_handle_cmd_login(hWnd, message, wParam, lParam);
			break;

		case ID_USR_SYSSET_BUTTON:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET), hWnd, systemSetProc); 
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int ui_handle_paint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	char text[32] = "注册账号";
	
	hdc = BeginPaint(hWnd, &ps); //lff:获取窗口客户区无效区域的设备环境句柄(代表屏幕上一块区域)

	SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);//设置为透明模式
	COLORREF color = SetTextColor(hdc, RGB(50,150,200));
	TextOut(hdc, 310, 100, text, strlen(text));
	SetTextColor(hdc, color);
	
	EndPaint(hWnd, &ps);
	
	return 0;
}

int ui_handle_keydown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{  
        case VK_LEFT://左方向键  
            break;  
			
        case VK_RIGHT:  
            break; 
			
        case VK_UP:
            break; 
			
        case VK_DOWN:
            break; 

		case VK_SPACE: //空格键
			break;
			
        default:  
            break;
	}
	return 0;
}


int ui_handle_char(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	return 0;
}

int ui_handle_lbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	wsprintf(szPoint, "QQ 2009 (X=%d, Y=%d)", x, y);
	SetWindowText(hWnd, szPoint);

	if((305 < x)&&(370 > x)&&(100 < y)&&(120 > y))
	{
		SendMessage(hWnd,WM_CLOSE,0,0);
	}
	return 0;
}


int ui_handle_rbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ui_handle_close(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	set_ui_Qclient_hwnd((HWND)INVALID_HWND);
	KillTimer(hWnd,MY_TIMEER);
	DestroyWindow(hWnd);
	
	return 0;
}


//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			ui_handle_create(hWnd, message, wParam, lParam);
			break;

		case WM_TIMER:
			ui_handle_timer(hWnd, message, wParam, lParam);
			break;

		case WM_COMMAND:
			ui_handle_command(hWnd, message, wParam, lParam);
			break;

		case WM_PAINT:
			ui_handle_paint(hWnd, message, wParam, lParam);
			break;

		case WM_USER_CHANGED:
			ui_handle_usr_changed(hWnd, message, wParam, lParam);
			break;
			
		case WM_KEYDOWN:
			ui_handle_keydown(hWnd, message, wParam, lParam);
			break;

		case WM_CHAR: //lff: WM_KENDOWN消息被TranslateMessage转化为WM_CHAR消息
			ui_handle_char(hWnd, message, wParam, lParam);
			break;

		case WM_LBUTTONDOWN: //lff:鼠标左键按下消息
			ui_handle_lbuttondown(hWnd, message, wParam, lParam);
			break;

		case WM_RBUTTONDOWN: //lff:鼠标右键按下
			ui_handle_rbuttondown(hWnd, message, wParam, lParam);
			break;

		case WM_CLOSE:
			ui_handle_close(hWnd, message, wParam, lParam);
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
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCE(IDC_WIN32QCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW&~WS_SIZEBOX&~WS_MAXIMIZEBOX, //&~WS_MINIMIZEBOX
      CW_USEDEFAULT, 0, 430, 330, NULL, NULL, hInstance, NULL);  //设置窗口风格为不能改变大小,无最大化按钮

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32QCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32QCLIENT));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

