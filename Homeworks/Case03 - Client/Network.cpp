#include "stdafx.h"
#include "Network.hpp"

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

void ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}
