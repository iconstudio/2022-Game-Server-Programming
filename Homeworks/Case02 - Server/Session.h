#pragma once

class Session
{
public:
	Session(ServerFramework* nframework);

	void ClearRecvBuffer();
	void ClearSendBuffer();
	void ClearOverlap(LPWSAOVERLAPPED overlap);

	void ReceiveStartPosition();
	Player* CreatePlayerCharacter();

	Player* Instance;
	ServerFramework* Framework;

	INT ID;
	SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	LPWSAOVERLAPPED Overlap_recv, Overlap_send;
	WSABUF Buffer_recv, Buffer_send;
	ULONG Size_recv, Size_send;
	char CBuffer_recv[BUFFSIZE], CBuffer_send[BUFFSIZE];
};
