#ifndef COMSUBDEFINE_H
#define COMSUBDEFINE_H

//用户自定义消息
#define WM_USER_CHANGED    (WM_USER+1)
#define WM_USER_RECV_DATA  (WM_USER+2)


#define INVALID_HWND  0xFFFFFFFF

void set_ui_Qclient_hwnd(HWND hWnd);
HWND get_ui_Qclient_hwnd(void);
void set_ui_Qclient_afterLogin_hwnd(HWND hWnd);
HWND get_ui_Qclient_afterLogin_hwnd(void);
void set_ui_Qclient_chat_hwnd(HWND hWnd, int i, int clientNo, char *data);
HWND get_ui_Qclient_chat_hwnd(unsigned int clientNo);
char *get_ui_talkData_by_hwnd(HWND hwnd);


#endif
