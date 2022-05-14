#pragma once
#include "stdafx.hpp"

class Asynchron : public WSAOVERLAPPED
{
public:
	Asynchron(ASYNC_OPERATIONS operation);
	~Asynchron();

	void SetSendBuffer(const WSABUF& buffer);
	void SetSendBuffer(LPWSABUF buffer);
	void SetSendBuffer(CHAR* cbuffer, DWORD size);

	const ASYNC_OPERATIONS Operation;
	PACKET_TYPES Type;

	std::unique_ptr<WSABUF> sendBuffer;
	std::unique_ptr<CHAR> sendCBuffer;
	DWORD sendSize, sendSzWant;
};

enum class ASYNC_OPERATIONS : UCHAR
{
	NONE = 0,
	RECV,
	SEND,
};
