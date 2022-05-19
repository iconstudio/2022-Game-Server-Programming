#pragma once
#include "stdafx.hpp"

class Asynchron : public WSAOVERLAPPED
{
public:
	Asynchron(OVERLAP_OPS operation);
	~Asynchron();

	void SetSendBuffer(const WSABUF& buffer);
	void SetSendBuffer(LPWSABUF buffer);
	void SetSendBuffer(CHAR* cbuffer, DWORD size);

	const OVERLAP_OPS Operation;
	PACKET_TYPES Type;

	std::unique_ptr<WSABUF> sendBuffer;
	std::unique_ptr<CHAR> sendCBuffer;
	DWORD sendSize, sendSzWant;
};

enum class OVERLAP_OPS : UCHAR
{
	NONE = 0,
	RECV,
	SEND,
};

void ClearOverlap(LPWSAOVERLAPPED overlap);
