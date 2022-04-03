#include "stdafx.h"

TCHAR* Msg_buffer = NULL;

EXOVERLAPPED::EXOVERLAPPED(OVERLAP_OPS operation)
	: Operation(operation), Type()
	, recvBuffer(), recvCBuffer(), recvSize(0), recvSzWant(0)
	, sendBuffer(), sendCBuffer(), sendSize(0), sendSzWant(0)
{}

void EXOVERLAPPED::SetRecvBuffer(WSABUF& buffer)
{
	*recvBuffer = buffer;
	recvSzWant = buffer.len;
}

void EXOVERLAPPED::SetRecvBuffer(LPWSABUF buffer)
{
	SetRecvBuffer(*buffer);
}

void EXOVERLAPPED::SetRecvBuffer(CHAR* cbuffer, DWORD size)
{
	if (!recvBuffer) recvBuffer = new WSABUF;

	recvBuffer->buf = cbuffer;
	recvBuffer->len = size;
	recvSzWant = size;
}

void EXOVERLAPPED::SetSendBuffer(WSABUF& buffer)
{
	*sendBuffer = buffer;
	sendSzWant = buffer.len;
}

void EXOVERLAPPED::SetSendBuffer(LPWSABUF buffer)
{
	SetSendBuffer(*buffer);
}

void EXOVERLAPPED::SetSendBuffer(CHAR* cbuffer, DWORD size)
{
	if (!sendBuffer) sendBuffer = new WSABUF;

	sendBuffer->buf = cbuffer;
	sendBuffer->len = size;
	sendSzWant = size;
}

Packet::Packet(PACKET_TYPES type, USHORT size, PID pid)
	: Type(type), Size(size), playerID(pid)
{}

Packet::Packet(PACKET_TYPES type, PID pid)
	: Packet(type, sizeof(Packet), pid)
{}

CSPacketSignIn::CSPacketSignIn(const CHAR* nickname)
	: Packet(PACKET_TYPES::CS_SIGNIN, sizeof(CSPacketSignIn), 0)
	, Nickname()
{
	ZeroMemory(Nickname, sizeof(Nickname));
	strcpy_s(Nickname, nickname);
}

CSPacketSignOut::CSPacketSignOut(PID pid)
	: Packet(PACKET_TYPES::CS_SIGNOUT, pid)
{}

CSPacketKeyInput::CSPacketKeyInput(PID pid, WPARAM key)
	: Packet(PACKET_TYPES::CS_KEY, sizeof(CSPacketKeyInput), pid)
	, Key(key)
{}

SCPacketSignUp::SCPacketSignUp(PID nid, UINT users, UINT usersmax)
	: Packet(PACKET_TYPES::SC_SIGNUP, sizeof(SCPacketSignUp), nid)
	, usersCurrent(users), usersMax(usersmax)
{}

SCPacketCreateCharacter::SCPacketCreateCharacter(PID pid, CHAR cx, CHAR cy)
	: Packet(PACKET_TYPES::SC_CREATE_CHARACTER, sizeof(SCPacketCreateCharacter), pid)
	, x(cx), y(cy)
{}

SCPacketMoveCharacter::SCPacketMoveCharacter(PID pid, CHAR nx, CHAR ny)
	: Packet(PACKET_TYPES::SC_MOVE_CHARACTER, sizeof(SCPacketMoveCharacter), pid)
	, x(nx), y(ny)
{}

SCPacketSignOut::SCPacketSignOut(PID pid)
	: Packet(PACKET_TYPES::SC_SIGNOUT, pid)
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
