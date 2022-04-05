#include "stdafx.h"

TCHAR* Msg_buffer = NULL;

bool PlayerCharacter::TryMoveLT()
{
	if (0 < x)
	{
		x--;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveRT()
{
	if (x < CELLS_CNT_H - 1)
	{
		x ++;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveUP()
{
	if (0 < y)
	{
		y--;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveDW()
{
	if (y < CELLS_CNT_V - 1)
	{
		y++;
		return true;
	}
	return false;
}

void ErrorDisplay(const char* title)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(TCHAR*)&Msg_buffer, 0, NULL);

	std::cout << title << " -> ¿À·ù: ";
	setlocale(LC_ALL, "KOREAN");
	wprintf(L"%s\n", Msg_buffer);

	LocalFree(Msg_buffer);
}
