#pragma once
#include "stdafx.h"

class ServerFramework;
struct Position;
class Player;

class Session
{
public:
	Session(ServerFramework* nframework, SOCKET sock);
	~Session();

	void ClearRecvBuffer();
	void ClearSendBuffer();
	void ClearOverlap(LPWSAOVERLAPPED overlap);

	void ReceiveStartPosition(DWORD begin_bytes = 0);
	void ProceedStartPosition(DWORD recv_bytes);
	Player* CreatePlayerCharacter();
	void ReceiveKeyInput(DWORD begin_bytes = 0);
	void ProceedKeyInput (DWORD recv_bytes);
	bool TryMove(WPARAM input);
	void SendWorld(LPWSABUF world_info, DWORD send_bytes = 0);
	void ProceedWorld(DWORD send_bytes);

	INT ID;

	Player* Instance;
	ServerFramework* Framework;

private:
	const SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	LPWSAOVERLAPPED Overlap_recv, Overlap_send;
	WSABUF Buffer_recv, Buffer_send;
	ULONG Size_recv, Size_send;
	char CBuffer_recv[BUFFSIZE], CBuffer_send[BUFFSIZE];
};
