#include "pch.hpp"
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"

Asynchron::Asynchron(ASYNC_OPERATIONS operation, PACKET_TYPES type)
	: myOperation(operation), myPacketType(type)
	, sendBuffer(nullptr), sendCBuffer(), sendSize(0), sendSzWant(0)
{}

Asynchron::~Asynchron()
{
	if (sendBuffer)
	{
		delete sendBuffer;
	}

	sendSzWant = 0;
}

void Asynchron::SetSendBuffer(const WSABUF& buffer)
{
	sendBuffer = new WSABUF(buffer);
	sendSzWant = buffer.len;
}

void Asynchron::SetSendBuffer(WSABUF&& buffer)
{
	const auto&& my_buffer = std::forward<WSABUF>(buffer);
	sendBuffer = new WSABUF(my_buffer);
	sendSzWant = my_buffer.len;
}

void Asynchron::SetSendBuffer(LPWSABUF buffer)
{
	sendBuffer = buffer;
	sendSzWant = buffer->len;
}

void Asynchron::SetSendBuffer(CHAR* cbuffer, DWORD size)
{
	if (!sendBuffer)
	{
		sendBuffer = new WSABUF();
	}

	sendBuffer->buf = cbuffer;
	sendBuffer->len = size;
	sendSzWant = size;
}

void ClearOverlap(WSAOVERLAPPED* overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void ClearOverlap(Asynchron* overlap)
{
	if (overlap->sendBuffer)
	{
		delete overlap->sendBuffer;
	}

	if (overlap->sendCBuffer)
	{
		delete[] overlap->sendCBuffer;
	}

	ZeroMemory(overlap, sizeof(Asynchron));
}
