#include "stdafx.hpp"
#include "Network.hpp"

Asynchron::Asynchron(OVERLAP_OPS operation)
	: Operation(operation), Type(PACKET_TYPES::NONE)
	, sendBuffer(nullptr), sendCBuffer(nullptr), sendSize(0), sendSzWant(0)
{}

Asynchron::~Asynchron()
{
	if (sendBuffer) sendBuffer.release();
	if (sendCBuffer) sendCBuffer.release();

	sendSzWant = 0;
}

void Asynchron::SetSendBuffer(const WSABUF& buffer)
{
	sendBuffer = std::make_unique<WSABUF>(buffer);
	sendSzWant = buffer.len;
}

void Asynchron::SetSendBuffer(LPWSABUF buffer)
{
	sendBuffer = std::make_unique<WSABUF>(*buffer);
	sendSzWant = buffer->len;
}

void Asynchron::SetSendBuffer(CHAR* cbuffer, DWORD size)
{
	if (!sendBuffer)
	{
		sendBuffer = std::make_unique<WSABUF>(WSABUF());
	}

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

SCPacketSignOut::SCPacketSignOut(PID pid, UINT users)
	: Packet(PACKET_TYPES::SC_SIGNOUT, pid)
	, usersCurrent(users)
{}

void ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}
