// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include "resource.h"
#include "RecevMessage.h"
#include "utils.h"
#include <stdio.h>
#include <WINSOCK2.H>
#include "socketTool.h"
#include <ctime> 

SOCKET Global_Client = 0;  //全局变量Global_Client的定义，其已在socketTool.h中声明

INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
VOID ShowUI(HMODULE hModule);
VOID hold_the_socket();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		while (Global_Client == 0)
		{
			Global_Client = Connect_to_Server(); //启动连接服务器
			if (Global_Client == 0)
				MessageBox(NULL, L"连接Python server失败", L"Connect server error", 0);
		}
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowUI, hModule, NULL, 0);
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)hold_the_socket, NULL, NULL, 0);
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Listen_to_Server, (LPVOID)client, NULL, 0);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//监听python server端发来的数据
VOID Listen_to_Server()
{
	SOCKET client = 0;
	while (true) {
		client = Global_Client;
		if (client != 0)
		{
			wchar_t buff[0x1000] = { 0 };
			CHAR recData[0x2000] = { 0 };
			DWORD ret = recv(client, recData, sizeof(recData), 0);
			if (ret > 0) {
				recData[ret] = 0x00;
				CharToUnicode(recData, buff);
				MessageBox(NULL, buff, L"Data from server", 0);
			}
			//closesocket(client);
		}
	}
}

//维持微信与server的socket连接
VOID hold_the_socket()
{
	wchar_t processPid[0x100] = { 0 };
	wchar_t buff[0x100] = { 0 };
	wchar_t type[0x100] = L"200";
	get_process_pid(processPid); //获取微信进程pid
	swprintf_s(buff, L"{\"pid\":%s,\"type\":%s,\"content\":\"%s\"}",
		processPid, type, L"heartbeat");
	const char * sendData = UnicodeToChar(buff);  //将Unicode编码转成CHAR类型，用于socket传输
	SOCKET client = 0;
	while (true)
	{
		
		client = Global_Client;
		//send()用来将数据由指定的socket传给对方主机
		//int send(int s, const void * msg, int len, unsigned int flags)
		//s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
		//成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error 
		int send_result = send(client, sendData, strlen(sendData), 0);
		if (send_result == -1)  //发送失败
		{
			closesocket(Global_Client);
			Global_Client = 0;
			while (Global_Client == 0)
			{
				Global_Client = Connect_to_Server(); //启动连接服务器，连接失败返回0
				Sleep(2 * 1000);  //延时5s
				//MessageBox(NULL, L"连接Python server失败", L"Connect server error", 0);
			}
		}
		Sleep(5 * 1000);  //延时5s
	}
}



//显示文本消息展示窗口
VOID ShowUI(HMODULE hModule)
{
	DialogBox(hModule, MAKEINTRESOURCE(MAIN), NULL, &DialogProc);
}

INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	DWORD hookAdd = getWechatWin() + 0x355613;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		StartHookWeChat(hwndDlg, hookAdd);
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:
		break;
	default:
		break;
	}
	return FALSE;
}
