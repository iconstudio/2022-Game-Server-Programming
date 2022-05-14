#include "stdafx.hpp"
#include "Asynchron.hpp"

TCHAR* msgBuffer = NULL;

void ClearOverlap(WSAOVERLAPPED* overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void ClearOverlap(Asynchron* overlap)
{
	if (overlap->sendBuffer)
	{
		overlap->sendBuffer.reset();
	}

	if (overlap->sendCBuffer)
	{
		overlap->sendCBuffer.reset();
	}

	ZeroMemory(overlap, sizeof(Asynchron));
}

void ErrorDisplay(const char* title)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(TCHAR*)&msgBuffer, 0, NULL);

	std::cout << title << " ¡æ ¿À·ù: ";
	wprintf(L"%s\n", msgBuffer);

	LocalFree(msgBuffer);
}
