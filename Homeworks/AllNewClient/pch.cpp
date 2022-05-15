#include "pch.hpp"

wchar_t* msgBuffer = NULL;
wchar_t msgTitle[512]{};

void ErrorAbort(const wchar_t* title, const int errorcode)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);

	ZeroMemory(msgTitle, sizeof(msgTitle));
	wsprintf(msgTitle, L"오류 → %s", title);

	MessageBox(NULL, msgBuffer, msgTitle, MB_ICONERROR | MB_OK);

	LocalFree(msgBuffer);
	exit(errorcode);
}

void ErrorDisplay(const wchar_t* title, const int errorcode)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);

	ZeroMemory(msgTitle, sizeof(msgTitle));
	wsprintf(msgTitle, L"오류 → %s", title);

	MessageBox(NULL, msgBuffer, msgTitle, MB_ICONERROR | MB_OK);

	LocalFree(msgBuffer);
}

void ErrorAbort(const wchar_t* title)
{
	ErrorAbort(title, WSAGetLastError());
}

void ErrorDisplay(const wchar_t* title)
{
	ErrorDisplay(title, WSAGetLastError());
}
