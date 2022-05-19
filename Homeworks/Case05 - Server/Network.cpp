#include "stdafx.hpp"
#include "Network.hpp"
#include "Packet.hpp"

EXOVERLAPPED::EXOVERLAPPED(OVERLAP_OPS operation)
	: Operation(operation), Type(PACKET_TYPES::NONE)
	, sendBuffer(nullptr), sendCBuffer(nullptr), sendSize(0), sendSzWant(0)
{}

EXOVERLAPPED::~EXOVERLAPPED()
{
	sendSzWant = 0;
}

void EXOVERLAPPED::SetSendBuffer(const WSABUF& buffer)
{
	sendBuffer = std::make_unique<WSABUF>(buffer);
	sendSzWant = buffer.len;
}

void EXOVERLAPPED::SetSendBuffer(LPWSABUF buffer)
{
	sendBuffer = std::make_unique<WSABUF>(*buffer);
	sendSzWant = buffer->len;
}

void EXOVERLAPPED::SetSendBuffer(CHAR* cbuffer, DWORD size)
{
	if (!sendBuffer)
	{
		sendBuffer = std::make_unique<WSABUF>(WSABUF());
	}

	sendBuffer->buf = cbuffer;
	sendBuffer->len = size;
	sendSzWant = size;
}

void ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}
