#pragma once
#include "stdafx.h"

void ClearOverlap(LPWSAOVERLAPPED overlap);

constexpr USHORT PORT = 6000;
constexpr UINT CLIENTS_MAX_NUMBER = 10;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
constexpr UINT BUFFSIZE = 512;

enum class OVERLAP_OPS : UCHAR
{
	NONE = 0,
	RECV,
	SEND,
};

class EXOVERLAPPED : public WSAOVERLAPPED
{
public:
	EXOVERLAPPED(OVERLAP_OPS operation);
	~EXOVERLAPPED();

	void SetSendBuffer(const WSABUF& buffer);
	void SetSendBuffer(LPWSABUF buffer);
	void SetSendBuffer(CHAR* cbuffer, DWORD size);

	const OVERLAP_OPS Operation;
	PACKET_TYPES Type;

	std::unique_ptr<WSABUF> sendBuffer;
	std::unique_ptr<CHAR> sendCBuffer;
	DWORD sendSize, sendSzWant;
};
