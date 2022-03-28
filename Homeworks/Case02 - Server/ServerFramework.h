#pragma once
#include "stdafx.h"

class Session;

DWORD WINAPI Communicate(LPVOID arg);

class Player : public Position
{
public:
	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};

class ServerFramework
{
public:
	ServerFramework();
	~ServerFramework();

	void Init();
	void Start();

	void AddClient(INT nid, Session* session);
	void AddClient(LPWSAOVERLAPPED overlap, Session* session);
	UINT GetClientsNumber() const;

	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);

	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemoveInstance(Position* instance);
	void RemoveSession(const INT id);

	void AddInstance(Position* instance);
	Position** GetInstancesData();

	SOCKET Socket;

private:
	void AcceptSession();
	void BroadcastWorld();
	friend DWORD WINAPI Communicate(LPVOID arg);

	SOCKADDR_IN Address;
	INT sz_Address;

	WSAOVERLAPPED Overlap;
	vector<Position*> World;

	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
	INT Clients_index , Clients_number;
};
