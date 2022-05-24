#include "pch.hpp"
#include "stdafx.hpp"
#include "Commons.hpp"
#include "Packet.hpp"

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

SCPacketCreatePlayer::SCPacketCreatePlayer(PID pid, const CHAR* nickname)
	: Packet(PACKET_TYPES::SC_CREATE_PLAYER, sizeof(SCPacketCreatePlayer), pid)
	, Nickname()
{
	ZeroMemory(Nickname, sizeof(Nickname));
	strcpy_s(Nickname, nickname);
}

SCPacketAppearCharacter::SCPacketAppearCharacter(PID cid, ENTITY_TYPES type, float nx, float ny)
	: Packet(PACKET_TYPES::SC_APPEAR_CHARACTER, sizeof(SCPacketAppearCharacter), cid)
	, myType(type), x(nx), y(ny)
{}

SCPacketMoveCharacter::SCPacketMoveCharacter(PID cid, float nx, float ny)
	: Packet(PACKET_TYPES::SC_MOVE_CHARACTER, sizeof(SCPacketMoveCharacter), cid)
	, x(nx), y(ny)
{}

SCPacketDisppearCharacter::SCPacketDisppearCharacter(PID cid)
	: Packet(PACKET_TYPES::SC_DISAPPEAR_CHARACTER, cid)
{}

SCPacketSignOut::SCPacketSignOut(PID pid, UINT users)
	: Packet(PACKET_TYPES::SC_SIGNOUT, sizeof(SCPacketSignOut), pid)
	, usersCurrent(users)
{}

CSPacketChatMessage::CSPacketChatMessage(PID fid, PID tid, const WCHAR* msg, size_t length)
	: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
	, Caption()
{
	ZeroMemory(Caption, sizeof(Caption));
	lstrcpyn(Caption, msg, length);
}

CSPacketChatMessage::CSPacketChatMessage(PID fid, PID tid, const WCHAR msg[])
	: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
	, Caption()
{
	ZeroMemory(Caption, sizeof(Caption));
	lstrcpy(Caption, msg);
}

SCPacketChatMessage::SCPacketChatMessage(PID tid, const WCHAR* msg, size_t length)
	: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
	, Caption()
{
	ZeroMemory(Caption, sizeof(Caption));
	lstrcpyn(Caption, msg, length);
}

SCPacketChatMessage::SCPacketChatMessage(PID tid, const WCHAR msg[])
	: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
	, Caption()
{
	ZeroMemory(Caption, sizeof(Caption));
	lstrcpy(Caption, msg);
}
