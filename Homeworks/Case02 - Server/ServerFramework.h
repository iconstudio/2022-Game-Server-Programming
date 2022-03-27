#pragma once
#include "stdafx.h"

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
	Position* GetWorldInstanceData();

	SOCKET Socket;

private:
	void AcceptSession();

	SOCKADDR_IN Address;
	INT sz_Address;

	WSAOVERLAPPED Overlap;
	vector<Position*> World;
	vector<Position> World_blob;
	PacketInfo World_desc;

	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
	INT Clients_index = 0, Clients_number = 0;
};
