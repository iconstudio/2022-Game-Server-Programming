#pragma once
#include "stdafx.h"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void ClearRecvBuffer();

	void Send(DWORD begin_bytes = 0);
	void ReceiveStartPosition(DWORD begin_bytes = 0);

	const PID ID;
	const SOCKET Socket;

	UINT szRecv, szWantRecv;

	PlayerCharacter const* Instance;

private:
	WSAOVERLAPPED overlapRecv;
	WSABUF bufferRecv;
	char cbufferRecv[BUFSIZ];
	Packet recvPacketDescriptor;

	LPWSAOVERLAPPED overlapSend;

	const IOCPFramework const& Framework;
};
