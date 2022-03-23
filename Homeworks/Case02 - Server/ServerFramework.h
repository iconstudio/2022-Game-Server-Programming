#pragma once
#include "stdafx.h"

void CallbackStartPositions(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CallbackInputs(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

class ServerFramework
{
public:
	ServerFramework();
	~ServerFramework();

	void Init();
	void Start();
	void AddClient(INT nid, Session* session);
	void AddClient(LPWSAOVERLAPPED overlap, Session* session);
	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);
	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);

	SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	WSAOVERLAPPED Overlap_recv;
	WSABUF Buffer_recv;
	CHAR CBuffer_recv[BUFFSIZE];

private:
	void AcceptSession();
	void RemoveSession(const INT id);

	INT Clients_index = 0;
	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
};

struct Position
{
	int x, y;
};

class Player : public Position
{
public:
	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
