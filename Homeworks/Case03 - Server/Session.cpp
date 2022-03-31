#include "stdafx.h"
#include "Session.h"

Session::Session(PID id, SOCKET sock, IOCPServerFramework& framework)
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

void Session::ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

