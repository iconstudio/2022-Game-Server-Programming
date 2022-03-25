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
	void RemoveSession(const INT id);

	void AddInstance(Position* instance);
	void GenerateWorldData();
	void SendWorld(Session* session, DWORD send_bytes = 0);
	void ProceedWorld(Session* session, DWORD send_bytes);

	friend void CallbackWorld(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

	SOCKET Socket;

private:
	void AcceptSession();
	void BroadcastWorld();

	SOCKADDR_IN Address;
	INT sz_Address;

	WSAOVERLAPPED Overlap;
	ULONG Size_send;

	vector<Position*> World;
	WSABUF* World_blob;
	UINT World_blob_length;
	PacketInfo World_data_desc;
	CHAR* World_cbuffer;

	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
	INT Clients_index = 0, Clients_number = 0;
};
