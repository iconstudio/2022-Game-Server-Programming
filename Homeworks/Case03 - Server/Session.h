#pragma once
#include "stdafx.h"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPServerFramework& framework);
	~Session();

	void ClearRecvBuffer();
	void ClearOverlap(LPWSAOVERLAPPED overlap);

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

	const IOCPServerFramework const& Framework;
};
