// RecvMessage.cpp : 定义 DLL 应用程序的导出函数。
// 获取微信接收到的文本信息，并发给Server端
#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include "resource.h"
#include <TlHelp32.h>
#include "utils.h"
#include <stdlib.h>
#include <WINSOCK2.H>
#include "socketTool.h"
#pragma comment(lib, "ws2_32.lib")
#define HOOK_LEN 5

//SOCKET rece_client = 0;   //socket套接字
HANDLE hWHND = 0;
BYTE backCode[HOOK_LEN] = { 0 };
DWORD WinAdd = 0;
DWORD retAdd = 0;
HWND hDlg = 0;

VOID printLog(DWORD msgAdd);
VOID send_to_py_server(DWORD msgAdd);
VOID StartHookWeChat(HWND hwndDlg, DWORD hookAdd, SOCKET client);

/*
esi + 0x178 消息内容
esi + 0x1A0 如果是个人消息则是个人wxid如果是群消息则是群id
esi + 0xCC 如果是群消息则这里为群里发送消息人的wxid反之为0x0
*/
//显示数据
VOID printLog(DWORD msgAdd)
{
	DWORD wxidAdd = msgAdd - 0x1B8;
	DWORD wxid2Add = msgAdd - 0xCC;
	DWORD messageAdd = msgAdd - 0x190;
	wchar_t buff[0x1000] = { 0 };
	GetDlgItemText(hDlg, MESSAGE, buff, sizeof(buff));
	if (*(LPVOID *)wxid2Add <= 0x0) {
		swprintf_s(buff, L"wxid: %s\r\n消息内容:\r\n%s", *((LPVOID *)wxidAdd), *((LPVOID *)messageAdd));
		//swprintf_s(buff, L"ESI=%p wxid=%p wxid2=%p wxid2=%p\r\n", msgAdd, msgAdd - 0x1A0, *((LPVOID *)wxidAdd));
	}
	else {
		swprintf_s(buff, L"群ID: %s\r\n发送者ID: %s\r\n消息内容:\r\n%s", *((LPVOID *)wxidAdd), *((LPVOID *)wxid2Add), *((LPVOID *)messageAdd));
	}

	SetDlgItemText(hDlg, MESSAGE, buff);
}

// 把数据发送给python服务器
VOID send_to_py_server(DWORD msgAdd)
{
	DWORD wxidAdd = msgAdd - 0x1B8;     //若是群消息，这里是群id；若是个人消息，则是个人id
	DWORD wxid2Add = msgAdd - 0xCC;     //若是群消息，这里是发送群消息的人的id；若是个人消息，这里是0x0
	DWORD messageAdd = msgAdd - 0x190;  //消息内容
	wchar_t buff[0x1000] = { 0 };
	wchar_t type[0x100] = L"1";  //消息类型，1代表发送给python server的是文本消息，用于python server对数据的解析
	//CHAR pid_str[0x100] = { 0 };
	//DWORD PID = GetCurrentProcessId();
	//// 把DWORD(即int)类型转成wchat_t类型
	//_itoa_s(PID, pid_str, 10);
	wchar_t processPid[0x100] = { 0 };
	get_process_pid(processPid); //获取微信进程pid
	//swprintf(processPid, sizeof(processPid), L"%hs", pid_str);

	//MessageBox(NULL, (LPCWSTR)processPid, L"ProcessId", 0);
	if (*(LPVOID *)wxid2Add <= 0x0) {
		//获取个人消息
		swprintf_s(buff, L"{\"pid\":%s,\"type\":%s,\"chatroom_ID\":\"%s\",\"wx_ID\":\"%s\",\"content\":\"%s\"}",
			processPid, type, L"", *((LPVOID *)wxidAdd), *((LPVOID *)messageAdd));
	}
	else {
		//获取群消息
		swprintf_s(buff, L"{\"pid\":%s,\"type\":%s,\"chatroom_ID\":\"%s\",\"wx_ID\":\"%s\",\"content\":\"%s\"}",
			processPid, type, *((LPVOID *)wxidAdd), *((LPVOID *)wxid2Add), *((LPVOID *)messageAdd));
	}
	//MessageBox(NULL, buff, L"Message", 0);

	if (Global_Client == 0)
		return;

	const char * sendData = UnicodeToChar(buff);  //将Unicode编码转成CHAR类型，用于socket传输
    //send()用来将数据由指定的socket传给对方主机
	//int send(int s, const void * msg, int len, unsigned int flags)
	//s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
	//成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error 
	send(Global_Client, sendData, strlen(sendData), 0);
	//closesocket(sclient);
}


DWORD cEax = 0;
DWORD cEcx = 0;
DWORD cEdx = 0;
DWORD cEbx = 0;
DWORD cEsp = 0;
DWORD cEbp = 0;
DWORD cEsi = 0;
DWORD cEdi = 0;

//跳转过来的函数 我们自己的
VOID __declspec(naked) HookF()
{
	//pushad: 将所有的32位通用寄存器压入堆栈
	//pushfd:然后将32位标志寄存器EFLAGS压入堆栈
	//popad:将所有的32位通用寄存器取出堆栈
	//popfd:将32位标志寄存器EFLAGS取出堆栈
	//先保存寄存器
	// 使用pushad这些来搞还是不太稳定  还是用变量把寄存器的值保存下来 这样可靠点
	__asm {
		mov cEax, eax
		mov cEcx, ecx
		mov cEdx, edx
		mov cEbx, ebx
		mov cEsp, esp
		mov cEbp, ebp
		mov cEsi, esi
		mov cEdi, edi

		pushad
		pushfd
	}
	//然后跳转到我们自己的处理函数
	//显示接收到的消息内容
	printLog(cEsi);
	//把数据发给server
	send_to_py_server(cEsi);
	retAdd = WinAdd + 0x355618;
	//然后在还原他进来之前的所有数据
	/*popad
		popfd  不太可靠恢复不全 所以才有变量的方式保存下来再赋值过去*/
	__asm {
		/*mov eax, cEax
		mov ecx, cEcx
		mov edx, cEdx
		mov ebx, cEbx
		mov esp, cEsp
		mov ebp, cEbp
		mov esi, cEsi
		mov edi, cEdi
		mov eax, retCallAdd
		call retCallAdd2*/
		popfd    //把备份的数据恢复
		popad
		jmp retAdd
	}
}

VOID StartHookWeChat(HWND hwndDlg, DWORD hookAdd)
{
	hWHND = OpenProcess(PROCESS_ALL_ACCESS, NULL, GetCurrentProcessId());
	WinAdd = getWechatWin();
	hDlg = hwndDlg;
	LPVOID jmpAdd = &HookF;

	BYTE JmpCode[HOOK_LEN] = { 0 };
	//我们需要组成一段这样的数据
	// E9 11051111(这里是跳转的地方这个地方不是一个代码地址 而是根据hook地址和跳转的代码地址的距离计算出来的)
	JmpCode[0] = 0xE9;
	//计算跳转的距离公式是固定的
	//计算公式为 跳转的地址(也就是我们函数的地址) - hook的地址 - hook的字节长度
	*(DWORD *)&JmpCode[1] = (DWORD)jmpAdd - hookAdd - HOOK_LEN;

	//hook第二步 先备份将要被我们覆盖地址的数据 长度为我们hook的长度 HOOK_LEN 5个字节

	//获取进程句柄

	wchar_t debugBuff[0x100] = { 0 };
	swprintf_s(debugBuff, L"hook地址=%p  进程句柄=%p  jmp函数=%p  AA=%p", hookAdd, hWHND, jmpAdd, &HookF);
	//MessageBox(NULL, debugBuff, L"测试", MB_OK);
	//备份数据
	if (ReadProcessMemory(hWHND, (LPVOID)hookAdd, backCode, HOOK_LEN, NULL) == 0) {

		swprintf_s(debugBuff, L"hook地址=%p  进程句柄=%p  错误类型=%d", hookAdd, hWHND, GetLastError());
		MessageBox(NULL, debugBuff, L"读取失败", MB_OK);
		//MessageBox(NULL,"hook地址的数据读取失败","读取失败",MB_OK);
		return;
	}

	//真正的hook开始了 把我们要替换的函数地址写进去 让他直接跳到我们函数里面去然后我们处理完毕后再放行吧！
	if (WriteProcessMemory(hWHND, (LPVOID)hookAdd, JmpCode, HOOK_LEN, NULL) == 0) {
		MessageBox(NULL, L"hook写入失败，函数替换失败", L"错误", MB_OK);
		return;
	}
	//MessageBox(NULL, L"成功HOOK", L"成功", MB_OK);
}