#include "pch.hpp"
#include "stdafx.hpp"
#include "Commons.hpp"
#include "Packet.hpp"

CSPacketSignIn::CSPacketSignIn(const CHAR* nickname)
	: Packet(PACKET_TYPES::CS_SIGNIN, sizeof(CSPacketSignIn), 0)
	, Nickname()
{
	strcpy_s(Nickname, nickname);
}

CSPacketSignOut::CSPacketSignOut(PID pid)
	: Packet(PACKET_TYPES::CS_SIGNOUT, pid)
{}

CSPacketMove::CSPacketMove(PID pid, UCHAR key)
	: Packet(PACKET_TYPES::CS_MOVE, sizeof(CSPacketMove), pid)
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
	strcpy_s(Nickname, nickname);
}

SCPacketAppearCharacter::SCPacketAppearCharacter(PID cid, ENTITY_TYPES type, float nx, float ny)
	: Packet(PACKET_TYPES::SC_APPEAR_OBJ, sizeof(SCPacketAppearCharacter), cid)
	, myType(type), x(nx), y(ny)
{}

SCPacketMoveCharacter::SCPacketMoveCharacter(PID cid, float nx, float ny)
	: Packet(PACKET_TYPES::SC_MOVE_OBJ, sizeof(SCPacketMoveCharacter), cid)
	, x(nx), y(ny)
{}

SCPacketDisppearCharacter::SCPacketDisppearCharacter(PID cid)
	: Packet(PACKET_TYPES::SC_DISAPPEAR_OBJ, cid)
{}

SCPacketSignOut::SCPacketSignOut(PID pid, UINT users)
	: Packet(PACKET_TYPES::SC_SIGNOUT, sizeof(SCPacketSignOut), pid)
	, usersCurrent(users)
{}

CSPacketChatMessage::CSPacketChatMessage(PID fid, PID tid, const WCHAR* msg, size_t length)
	: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
	, Caption(), targetID(tid)
{
	auto wstr = lstrcpyn(Caption, msg, int(length));
}

CSPacketChatMessage::CSPacketChatMessage(PID fid, PID tid, const WCHAR msg[])
	: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
	, Caption()
{
	lstrcpy(Caption, msg);
}

SCPacketChatMessage::SCPacketChatMessage(PID tid, const WCHAR* msg, size_t length)
	: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
	, Caption()
{
	auto wstr = lstrcpyn(Caption, msg, int(length));
}

SCPacketChatMessage::SCPacketChatMessage(PID tid, const WCHAR msg[])
	: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
	, Caption()
{
	lstrcpy(Caption, msg);
}
