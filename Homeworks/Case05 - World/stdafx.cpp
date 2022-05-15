#include "stdafx.h"

TCHAR* msgBuffer = NULL;

PlayerCharacter::PlayerCharacter(INT ox, INT oy) : x(ox), y(oy)
{}

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
		(TCHAR*)&msgBuffer, 0, NULL);

	std::cout << title << " ¡æ ¿À·ù: ";
	wprintf(L"%s\n", msgBuffer);

	LocalFree(msgBuffer);
}
