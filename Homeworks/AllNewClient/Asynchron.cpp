#include "pch.hpp"
#include "Asynchron.hpp"

Asynchron::Asynchron(ASYNC_OPERATIONS operation)
	: myOperation(operation)
	, sendBuffer(nullptr), sendCBuffer(), sendSize(0), sendSzWant(0)
{}

Asynchron::~Asynchron()
{

	ClearOverlap(this);
	sendSzWant = 0;
}

void Asynchron::SetSendBuffer(const WSABUF& buffer)
{
	sendBuffer = new WSABUF(buffer);
	sendSzWant = buffer.len;
}

void Asynchron::SetSendBuffer(LPWSABUF buffer)
{
	sendBuffer = new WSABUF(*buffer);
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
