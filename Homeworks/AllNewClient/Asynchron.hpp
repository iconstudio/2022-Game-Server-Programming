#pragma once

class Asynchron : public WSAOVERLAPPED
{
public:
	Asynchron(ASYNC_OPERATIONS operation, PACKET_TYPES type = PACKET_TYPES(0));
	virtual ~Asynchron();

	void SetSendBuffer(const WSABUF& buffer);
	void SetSendBuffer(WSABUF&& buffer);
	void SetSendBuffer(LPWSABUF buffer);
	void SetSendBuffer(CHAR* cbuffer, DWORD size);

	const ASYNC_OPERATIONS myOperation;
	const PACKET_TYPES myPacketType;

	LPWSABUF sendBuffer;
	CHAR sendCBuffer[BUFFSZ];
	DWORD sendSize, sendSzWant;
};

enum class ASYNC_OPERATIONS : UCHAR
{
	NONE = 0,
	RECV,
	SEND,
};

void ClearOverlap(WSAOVERLAPPED* overlap);
void ClearOverlap(Asynchron* overlap);
