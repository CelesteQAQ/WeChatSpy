#pragma once
#include "pch.h"
#include<WINSOCK2.H>

//获取wechatWin模块地址
DWORD getWechatWin();
//获取微信进程pid
VOID get_process_pid(wchar_t * processPid);

//将wchat_t类型数组转成CHAR类型数组
char* UnicodeToChar(const wchar_t* unicode);
//Char类型数组转wchat_t类型数组
VOID CharToUnicode(CHAR * charList, wchar_t * wchar_t_list);
// 把DWORD(即int)类型变量转成wchat_t类型数组
VOID DWORDToUnicode(DWORD value, wchar_t * wchar_t_list);