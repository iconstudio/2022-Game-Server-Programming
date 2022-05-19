#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"

Asynchron::Asynchron(OVERLAP_OPS operation)
	: Operation(operation), Type(PACKET_TYPES::NONE)
	, sendBuffer(nullptr), sendCBuffer(nullptr), sendSize(0), sendSzWant(0)
{}

Asynchron::~Asynchron()
{
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

void ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}
