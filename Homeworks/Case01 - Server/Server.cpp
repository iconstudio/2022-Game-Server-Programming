#include "stdafx.h"
#include "Server.h"
#include "ServerFramework.h"

ServerFramework framework;

int main()
{
	framework.Init();
	framework.Start();

	cout << "서버 종료됨\n";
	while (true)
	{
	}

	return 0;
}

void ErrorDisplay(const char* title)
{
	TCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	wcout << title << " -> 오류: " << lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}
