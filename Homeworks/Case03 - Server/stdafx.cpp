#include "stdafx.h"

TCHAR* Msg_buffer = NULL;

Packet::Packet(PACKET_TYPES type, USHORT size, PID pid)
	: Type(type), Size(size), playerID(pid)
{}

Packet::Packet(PACKET_TYPES type, PID pid)
	: Packet(type, sizeof(Packet), pid)
{}

CSPacketSignIn::CSPacketSignIn(const CHAR* nickname)
	: Packet(CS_SIGNIN, sizeof(CSPacketSignIn), 0)
	, Nickname()
{
	ZeroMemory(Nickname, sizeof(Nickname));
	strcpy_s(Nickname, nickname);
}

CSPacketSignOut::CSPacketSignOut(PID pid)
	: Packet(CS_SIGNOUT, pid)
{}

CSPacketKeyInput::CSPacketKeyInput(PID pid, WPARAM key)
	: Packet(CS_KEY, sizeof(CSPacketKeyInput), pid), Key(key)
{}

SCPacketSignUp::SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0)
	: Packet(SC_SIGNUP, sizeof(SCPacketSignUp), nid)
	, usersCurrent(users), usersMax(usersmax)
{}

SCPacketCreateCharacter::SCPacketCreateCharacter(PID pid, UCHAR cx, UCHAR cy)
	: Packet(SC_CREATE_CHARACTER, sizeof(SCPacketCreateCharacter), pid)
	, x(cx), y(cy)
{}

SCPacketMoveCharacter::SCPacketMoveCharacter(PID pid, UCHAR nx, UCHAR ny)
	: Packet(SC_MOVE_CHARACTER, sizeof(SCPacketMoveCharacter), pid)
	, x(nx), y(ny)
{}

SCPacketSignOut::SCPacketSignOut(PID pid)
	: Packet(SC_SIGNOUT, pid)
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

void ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

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
