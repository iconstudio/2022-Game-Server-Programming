#include "stdafx.h"
#include "Session.h"
#include "ServerFramework.h"

Session::Session(ServerFramework* nframework)
	: framework(nframework)
	, Overlap_recv(new WSAOVERLAPPED()), Overlap_send(new WSAOVERLAPPED())
	, Buffer_recv(), Buffer_send(), Size_recv(0), Size_send(0)
{
	ClearOverlap(Overlap_recv);
	ClearOverlap(Overlap_send);
	ClearRecvBuffer();
	ClearSendBuffer();

	framework->AddClient(Overlap_recv, this);
	framework->AddClient(Overlap_send, this);

	Buffer_recv.buf = CBuffer_recv;
	Buffer_recv.len = BUFFSIZE;
	Buffer_send.buf = CBuffer_send;
	Buffer_send.len = BUFFSIZE;
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(CBuffer_recv, BUFFSIZE);
}

void Session::ClearSendBuffer()
{
	ZeroMemory(CBuffer_send, BUFFSIZE);
}

void Session::ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void Session::ReceiveStartPosition()
{
	cout << "Client " << ID << " is receving its position.\n";
	DWORD recv_flag = 0;
	int result = WSARecv(Socket, &Buffer_recv, 1, 0, &recv_flag
		, NULL, CallbackStartPositions);
	if (SOCKET_ERROR == result)
	{
		ErrorDisplay("WSARecv 1");
		return;
	}
}

Player* Session::CreatePlayerCharacter()
{
	auto instance = new Player;
	Instance = instance;

	return (instance);
}
