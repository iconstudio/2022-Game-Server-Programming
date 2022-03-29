#include "stdafx.h"

TCHAR* Msg_buffer = NULL;

void ErrorDisplay(const char* title)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(TCHAR*)&Msg_buffer, 0, NULL);

	cout << title << " -> ¿À·ù: ";
	setlocale(LC_ALL, "KOREAN");
	wprintf(L"%s\n", Msg_buffer);

	LocalFree(Msg_buffer);
}
