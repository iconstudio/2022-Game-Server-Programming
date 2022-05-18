#include "pch.hpp"
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"

Asynchron::Asynchron(ASYNC_OPERATIONS operation, PACKET_TYPES type)
	: myOperation(operation), myPacketType(type)
	, sendBuffer(nullptr), sendCBuffer(nullptr)
	, sendSize(0), sendSzWant(0)
{}

Asynchron::~Asynchron()
{
	sendSzWant = 0;
}

void Asynchron::SetSendBuffer(const WSABUF& buffer)
{
	sendBuffer = make_unique<WSABUF>(buffer);
	sendSzWant = buffer.len;
}

void Asynchron::SetSendBuffer(WSABUF&& buffer)
{
	const auto&& my_buffer = std::forward<WSABUF>(buffer);
	sendBuffer = make_unique<WSABUF>(my_buffer);
	sendSzWant = my_buffer.len;
}

void Asynchron::SetSendBuffer(LPWSABUF buffer)
{
	sendBuffer = unique_ptr<WSABUF>(buffer);
	sendSzWant = buffer->len;
}

void Asynchron::SetSendBuffer(const char* cbuffer, const size_t size)
{
	sendBuffer = make_unique<WSABUF>();
	sendCBuffer = unique_ptr<char[BUFFSZ]>();
	auto ptr = sendCBuffer.get();

	size_t length = std::min(BUFFSZ, size);
	CopyMemory(ptr, cbuffer, length);

	sendBuffer->buf = *(ptr);
	sendBuffer->len = length;
	sendSzWant = length;
}

template<size_t _Size>
void Asynchron::SetSendBuffer(char cbuffer[_Size])
{
	sendBuffer = make_unique<WSABUF>();
	sendCBuffer = unique_ptr<char[BUFFSZ]>();

	size_t length = std::min(BUFFSZ, _Size);
	CopyMemory(sendCBuffer.get(), cbuffer, length);

	sendBuffer->buf = *(sendCBuffer.get());
	sendBuffer->len = length;
	sendSzWant = length;
}

void ClearOverlap(WSAOVERLAPPED* overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void ClearOverlap(Asynchron* overlap)
{
	if (overlap->sendBuffer)
	{
		overlap->sendBuffer.reset();
	}

	if (overlap->sendCBuffer)
	{
		overlap->sendCBuffer.reset();
	}

	ZeroMemory(overlap, sizeof(Asynchron));
}
