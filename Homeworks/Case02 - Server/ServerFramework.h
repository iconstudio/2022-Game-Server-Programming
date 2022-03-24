#pragma once
#include "stdafx.h"

class Session;

class ServerFramework
{
public:
	ServerFramework();
	~ServerFramework();

	void Init();
	void Start();
	UINT GetClientsNumber() const;
	void AddClient(INT nid, Session* session);
	void AddClient(LPWSAOVERLAPPED overlap, Session* session);
	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);
	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemoveSession(const INT id);

	SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	WSAOVERLAPPED Overlap;
	WSABUF Buffer;
	CHAR CBuffer[BUFFSIZE];

	friend void CallbackBroadcastWorld(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

private:
	void AcceptSession();
	void BroadcastWorld();

	INT Clients_index = 0;
	vector<WSABUF*> World;
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

class PacketPlayerSpace
{
public:
	UINT Size = 0;
	UINT Length = 0;
	LPWSABUF Buffers;
};
