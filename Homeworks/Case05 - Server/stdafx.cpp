#include "pch.hpp"
#include "stdafx.hpp"

TCHAR* msgBuffer = NULL;

enum class ENTITY_TYPES : UCHAR
{
	NONE, NPC, QUEEST_NPC, MOB, BOSS, PLAYER
};

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
