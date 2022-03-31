#include "stdafx.h"
#include "Session.h"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Socket(sock), Framework(framework)
	, overlapRecv(), szRecv(0), szWantRecv(0), bufferRecv(), cbufferRecv()
	, recvPacketDescriptor(NONE, id)
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&overlapRecv);
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(cbufferRecv, sizeof(cbufferRecv));
	szRecv = 0;
}
