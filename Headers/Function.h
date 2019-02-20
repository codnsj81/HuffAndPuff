#pragma once

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>


//// ���� �Լ� ���� ��� �� ����
//void err_quit(char *msg)
//{
//	LPVOID lpMsgBuf;
//
//	// char* to LPWSTR (0211)
//	wchar_t wtext[128];
//	mbstowcs(wtext, msg, strlen(msg) + 1);
//	LPWSTR ptr = wtext;
//	// --------------------------
//
//	FormatMessage(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//		NULL, WSAGetLastError(),
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPTSTR)&lpMsgBuf, 0, NULL);
//	MessageBox(NULL, (LPCTSTR)lpMsgBuf, /*(LPWSTR)msg*/wtext, MB_ICONERROR);
//	LocalFree(lpMsgBuf);
//
//	
//
//	exit(1);
//}
//
//// ���� �Լ� ���� ���
//void err_display(char *msg)
//{
//	LPVOID lpMsgBuf;
//	FormatMessage(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//		NULL, WSAGetLastError(),
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPTSTR)&lpMsgBuf, 0, NULL);
//	printf("[%s] %s", msg, (char *)lpMsgBuf);
//	LocalFree(lpMsgBuf);
//}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}