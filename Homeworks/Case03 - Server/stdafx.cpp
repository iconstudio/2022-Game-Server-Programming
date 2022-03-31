#include "stdafx.h"

TCHAR* Msg_buffer = NULL;

Packet::Packet(PACKET_TYPES type, USHORT size, UINT pid)
	: Type(type), Size(size), playerID(pid)
{}

Packet::Packet(PACKET_TYPES type, UINT pid)
	: Packet(type, sizeof(Packet), pid)
{}

CSPacketSignIn::CSPacketSignIn(const CHAR* nickname)
	: Packet(CS_SIGNIN, sizeof(CSPacketSignIn), 0)
	, Nickname()
{
	ZeroMemory(Nickname, sizeof(Nickname));
	strcpy_s(Nickname, nickname);
}

CSPacketSignOut::CSPacketSignOut(UINT pid)
	: Packet(CS_SIGNOUT, pid)
{}

CSPacketKeyInput::CSPacketKeyInput(UINT pid, WPARAM key)
	: Packet(CS_KEY, sizeof(CSPacketKeyInput), pid), Key(key)
{}

SCPacketSignUp::SCPacketSignUp(UINT nid)
	: Packet(SC_SIGNUP, nid)
{}

SCPacketCreateCharacter::SCPacketCreateCharacter(UINT pid, UCHAR cx, UCHAR cy)
	: Packet(SC_CREATE_CHARACTER, sizeof(SCPacketCreateCharacter), pid)
	, x(cx), y(cy)
{}

SCPacketMoveCharacter::SCPacketMoveCharacter(UINT pid, UCHAR nx, UCHAR ny)
	: Packet(SC_MOVE_CHARACTER, sizeof(SCPacketMoveCharacter), pid)
	, x(nx), y(ny)
{}

SCPacketSignUp::SCPacketSignUp(UINT pid)
	: Packet(SC_SIGNOUT, pid)
{}

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
