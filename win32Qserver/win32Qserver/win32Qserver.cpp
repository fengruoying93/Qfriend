// win32Qserver.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "win32Qserver.h"
#include "q_server.h"

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

#define snprintf(buf,len, format,...) _snprintf_s(buf, len, len-1, format, __VA_ARGS__)

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
char szPoint[64] = {0}; //点击坐标
static BOOL startServer = 0;
HFONT hFont; //自定义字体
char g_showText[512] = "Qserver..";
HWND QserverHwnd = 0;

void MyPrintf(void)
{
	SetWindowText(GetDlgItem(QserverHwnd, ID_USR_TEXT_STATIC), g_showText);
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
			CreateWindowEx(0, "static", "服务器端口:", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
				30, 40, 80, 20, hDlg, (HMENU)ID_USR_DLG_PORT_STATIC, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE); //设置控件字体

			CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD|WS_VISIBLE|ES_LEFT, 
				120, 40, 100, 20, hDlg, (HMENU)ID_USR_DLG_PORT_EDIT, hInst, NULL);
			SendDlgItemMessage(hDlg, ID_USR_DLG_PORT_EDIT,WM_SETFONT,(WPARAM)hFont,TRUE);

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


int ui_handle_create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	QserverHwnd = hWnd;
	
	SetWindowText(hWnd, "Qserver");
	hFont = ui_create_font(hWnd);
	
	CreateWindowEx(0, "button", "开启服务", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		100, 80, 80, 30, hWnd, (HMENU)ID_USR_START_BUTTON, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_START_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "button", "设置", WS_CHILD|WS_VISIBLE, //SS_SUNKEN, 
		230, 80, 80, 30, hWnd, (HMENU)ID_USR_SET_BUTTON, hInst, NULL);
	SendDlgItemMessage(hWnd, ID_USR_SET_BUTTON,WM_SETFONT,(WPARAM)hFont,TRUE);

	CreateWindowEx(0, "edit", "信息显示框", WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,
		10, 140, 390, 120, hWnd, (HMENU)ID_USR_TEXT_STATIC, hInst, NULL);
	SetWindowText(GetDlgItem(hWnd, ID_USR_TEXT_STATIC), g_showText);
	SendDlgItemMessage(hWnd, ID_USR_TEXT_STATIC,WM_SETFONT,(WPARAM)hFont,TRUE);
	
	return 0;
}

int ui_handle_timer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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

		case ID_CLIENT_INFO:
			DisplayClientInfo();
			break;

		case ID_USR_START_BUTTON:
			if(0 == startServer)
			{
				startServer = 1;
				#ifdef WIN32
				DWORD  threadId;
				if (NULL == CreateThread(NULL, 0, QserverTask, 0, 0, &threadId))
				#else 
				pthread_t ntid;
				if (0 != pthread_create(&ntid, NULL, QserverTask, NULL))
				#endif
				{
					snprintf(g_showText, sizeof(g_showText), "create QserverTask fail!\n");
				}
			}
			break;

		case ID_USR_SET_BUTTON:
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
	
	hdc = BeginPaint(hWnd, &ps); //lff:获取窗口客户区无效区域的设备环境句柄(代表屏幕上一块区域)
	
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
	wsprintf(szPoint, "Qserver (X=%d, Y=%d)", x, y);
	SetWindowText(hWnd, szPoint);
	
	return 0;
}


int ui_handle_rbuttondown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ui_handle_close(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DestroyWindow(hWnd);
	
	return 0;
}

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32QSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32QSERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX&~WS_SIZEBOX,
      CW_USEDEFAULT, 0, 430, 330, NULL, NULL, hInstance, NULL);

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

	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32QSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32QSERVER));

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


