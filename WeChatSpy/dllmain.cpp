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
#include <stdlib.h>
#include "sendMessage.h"
#include "cJSON.h"
#include <atlstr.h>
using namespace std;


SOCKET Global_Client = 0;  //全局变量Global_Client的定义，其已在socketTool.h中声明

INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
VOID ShowUI(HMODULE hModule);
VOID hold_the_socket();
VOID Listen_to_Server();


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
				MessageBox(NULL, L"首次连接Python server失败", L"Connect server error", 0);
		}
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowUI, hModule, NULL, 0);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)hold_the_socket, NULL, NULL, 0);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Listen_to_Server, NULL, NULL, 0);
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
	DWORD ret = 0;
	cJSON *json;
	while (true) {
		client = Global_Client;
		if (client)
		{
			CHAR recData[0x2000] = { 0 };
			wchar_t wxid[0x100] = { 0 };
			wchar_t at_wxid[0x100] = { 0 };
			wchar_t message[0x1000] = { 0 };
			//wchar_t buff[0x1000] = { 0 };
			ret = 0;
			//const char * data = UnicodeToChar(buff);
			ret = recv(client, recData, sizeof(recData), 0);
			if (strlen(recData) != 0) {
				recData[strlen(recData)] = 0x00;
				//Document document;
				//document.Parse(recData);
				////printf("content = %s\n", document["content"].GetString());
				//swprintf(message, sizeof(message), L"%hs", document["content"].GetString());
				//MessageBox(NULL, message, L"Data from server message", 0);
				//swprintf(buff, sizeof(buff), L"%hs", recData);
				//MultiByteToWideChar(CP_UTF8, 0, recData, -1, buff, sizeof(buff));
				//int length = strlen(recData);
				//CN2Unicode(recData, buff);
				//CN2Unicode(recData, buff);
				wchar_t * buff2 = UTF8ToUnicode(recData);
				//_itoa_s(length, pid_str, 10);
				//get_process_pid(processPid); //获取微信进程pid， GetCurrentProcessId不能在其他文件调用
				//swprintf(message, sizeof(message), L"%hs", pid_str);
				//swprintf_s(text, L"%s", buff);
				//CHAR * pid_str = UnicodeToChar(buff);
				//swprintf(text, sizeof(message), L"%hs", pid_str);
				/*MessageBox(NULL, buff, L"Data from server buff", 0);*/
				//CString  m_str(recData);
				////MessageBox(m_str);
				//MessageBox(NULL, m_str, L"recData", 0);
				//MessageBox(NULL, message, L"length", 0);
				//wchar_t buff2[0x1000] = L"{\"code\":1,\"wxid\":\"wxid_5d7paxx35o8h22\",\"at_wxid\":\"\", \"content\":\"python server\u6536\u5230\"}";
				//MessageBox(NULL, buff2, L"Data from server buff2", 0);
				json = cJSON_Parse(buff2); //解析成json形式
				//wchar_t * wxid = cJSON_GetObjectItem(json, L"wxid")->valuestring;  //获取键值内容
				swprintf_s(wxid, L"%s", cJSON_GetObjectItem(json, L"wxid")->valuestring);
				//wchar_t * at_wxid = cJSON_GetObjectItem(json, L"at_wxid")->valuestring;
				swprintf_s(at_wxid, L"%s", cJSON_GetObjectItem(json, L"at_wxid")->valuestring);
				//wchar_t * message = cJSON_GetObjectItem(json, L"content")->valuestring;
				swprintf_s(message, L"%s", cJSON_GetObjectItem(json, L"content")->valuestring);
				//swprintf_s(text, L"wx_ID:%s,at_wxid:%s, content:%s}", wxid, at_wxid, message);
				//wchar_t * wxid = cJSON_GetObjectItem(json, L"wxid")->valuestring;  //获取键值内容
				//wchar_t * at_wxid = cJSON_GetObjectItem(json, L"at_wxid")->valuestring;
				//wchar_t * message = cJSON_GetObjectItem(json, L"content")->valuestring;
				cJSON_Delete(json);
				//swprintf_s(text, L"wx_ID:%s,at_wxid:%s, content:%s", wxid, at_wxid, message);
				//MessageBox(NULL, text, L"Data from server text最后一次", 0);
				sendTextMessage(wxid, at_wxid, message);
			}
		}
	}
}

//维持微信与server的socket连接
VOID hold_the_socket()
{
	wchar_t buff[0x100] = { 0 };
	wchar_t type[0x100] = L"200";
	//获取微信进程pid
	CHAR pid_str[0x100] = { 0 };
	wchar_t processPid[0x100] = { 0 };
	DWORD PID = GetCurrentProcessId();  
	// 把DWORD(即int)类型转成wchat_t类型
	_itoa_s(PID, pid_str, 10);
	//get_process_pid(processPid); //获取微信进程pid， GetCurrentProcessId不能在其他文件调用
	swprintf(processPid, sizeof(processPid), L"%hs", pid_str);
	swprintf_s(buff, L"{\"pid\":%s,\"type\":%s}", processPid, type);
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
				Sleep(1 * 1000);  //延时1s
				//MessageBox(NULL, L"连接Python server失败", L"Connect server error", 0);
			}
		}
		Sleep(2 * 1000);  //延时2s
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
	switch (uMsg)
	{
	case WM_INITDIALOG:
		StartHookWeChat(hwndDlg);
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
