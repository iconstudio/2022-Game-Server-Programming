#pragma once
#include "stdafx.h"
#include <queue>

class Session;

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
	void AcceptSession();
	void BroadcastWorld();

	void AddClient(INT nid, Session* session);
	void AddClient(LPWSAOVERLAPPED overlap, Session* session);
	UINT GetClientsNumber() const;

	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);

	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemovePlayerInstance(Player* instance);
	void RemoveSession(const INT id);

	void AssignPlayerInstance(Player*& instance);
	Player* GetInstancesData(int index);

	SOCKET Socket;

private:
	SOCKADDR_IN Address;
	INT sz_Address;

	WSAOVERLAPPED Overlap;

	CRITICAL_SECTION Client_sect;
	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
	INT Clients_index, Clients_number;

	vector<Player*> PlayerInst_pool;
	UINT PlayerInst_index = 0;
};
