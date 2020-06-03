#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include "utils.h"

//定义消息内容结构体
struct wxStr
{
	wchar_t * pStr;
	int strLen;
	int strLen2;
};

//发送文本消息
VOID sendTextMessage(wchar_t * wxid, wchar_t * at_wxid, wchar_t * message)
{
	//发送消息call, 偏移地址：0x32A760
	DWORD sendCall = getWechatWin() + 0x32A760;

	//组装需要的数据格式
	//微信id
	wxStr pWxid = { 0 };
	pWxid.pStr = wxid;
	pWxid.strLen = wcslen(wxid);
	pWxid.strLen2 = wcslen(wxid) * 2;


	wxStr at_pWxid = { 0 };
	char * asmAt_Wxid = 0x0;  //@微信id地址的指针
	//if (wcslen(at_wxid)!=0)
	//{
	//	//微信@id
	//	at_pWxid.pStr = at_wxid;
	//	at_pWxid.strLen = wcslen(at_wxid);
	//	at_pWxid.strLen2 = wcslen(at_wxid) * 2;
	//	asmAt_Wxid = (char *)&at_pWxid.pStr;  //@微信id地址的指针
	//}

	//微信文本内容
	wxStr pMessage = { 0 };
	pMessage.pStr = message;
	pMessage.strLen = wcslen(message);
	pMessage.strLen2 = wcslen(message) * 2;

	/*-------微信发送文本消息call汇编代码-------------
		mov edx, dword ptr ss : [ebp - 0x50]      ; 发给的个人微信id或者群id地址指针
		lea eax, dword ptr ds : [ebx + 0x28]
		push 0x1                                  ; 0x1
		push eax                                   ; 0，如果是在群里@人，就是@的那个人的id地址的指针。
		push ebx                                  ; 消息内容地址指针
		lea ecx, dword ptr ss : [ebp - 0x8DC]     ; 0x8DC
		call WeChatWi.5A40A760                    ; 微信发送消息call
		add esp, 0xC                              ; 平衡堆栈，若是不加这行代码去平衡，则微信会崩溃。
	*/
	char * asmWxid = (char *)&pWxid.pStr;   //微信id地址的指针
	char * asmMessage = (char *)&pMessage.pStr;  //消息内容地址的指针

	char buff[0x8DC] = { 0 };
	//char buff2[0x28] = { 0 };

	__asm {
		mov edx, asmWxid
		//lea eax, buff2
		mov eax, asmAt_Wxid
		push 0x1
		push eax
		mov ebx, asmMessage
		push ebx
		lea ecx, buff
		call sendCall
		add esp, 0xC
	}
}