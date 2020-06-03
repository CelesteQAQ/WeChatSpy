// 定义一些常用的工具函数
#include "pch.h"
#include <stdlib.h>
#include <WINSOCK2.H>
#include <stdio.h>

//获取wechatWin模块地址
DWORD getWechatWin()
{
	return (DWORD)LoadLibrary(L"WeChatWin.dll");
}

////获取微信进程pid
//VOID get_process_pid(wchar_t * processPid)
//{
//	CHAR pid_str[0x100] = { 0 };
//	DWORD PID = GetCurrentProcessId();
//	// 把DWORD(即int)类型转成wchat_t类型
//	_itoa_s(PID, pid_str, 10);
//	swprintf(processPid, sizeof(processPid), L"%hs", pid_str);
//}
//将wchat_t类型数组转成CHAR类型数组
char* UnicodeToChar(const wchar_t* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char *szCHAR = (char*)malloc(len + 1);
	memset(szCHAR, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szCHAR, len, NULL, NULL);
	return szCHAR;
}

//Char类型数组转wchat_t类型数组
VOID CharToUnicode(CHAR * charList, wchar_t * wchar_t_list)
{
	swprintf(wchar_t_list, sizeof(wchar_t_list), L"%hs", charList);
}

// 把DWORD(即int)类型变量转成wchat_t类型数组
VOID DWORDToUnicode(DWORD value, wchar_t * wchar_t_list)
{
	CHAR char_str[0x100] = { 0 };
	_itoa_s(value, char_str, 10);
	swprintf(wchar_t_list, sizeof(wchar_t_list), L"%hs", char_str);
}

