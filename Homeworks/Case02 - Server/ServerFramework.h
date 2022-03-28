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

	Session* GetClientByIndex(INT nth);
	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);

	void RemoveClient(INT rid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemoveSession(const INT id);

	void AssignPlayerInstance(shared_ptr<Player>& instance);
	Player* GetInstancesData(INT index);

	SOCKET Socket;

private:
	SOCKADDR_IN Address;
	INT sz_Address;

	CRITICAL_SECTION Client_sect;
	WSAOVERLAPPED Overlap;
	vector<INT> IndexedClients;
	unordered_map<INT, Session*> ClientsDict;
	unordered_map<LPWSAOVERLAPPED, Session*> ClientsOverlap;
	INT Clients_index, Clients_number;

	vector<Player*> Players_pool;
	UINT PlayerInst_index;
};
