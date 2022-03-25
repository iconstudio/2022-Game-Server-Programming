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

struct PacketInfo
{
	ULONG Size = 0;
	ULONG Length = 0;
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
	void AddPlayerSpace(Player* instance);
	UINT GetClientsNumber() const;

	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);

	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemoveSession(const INT id);

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

	vector<WSABUF> World;
	WSABUF* World_data;
	PacketInfo World_data_info;
	CHAR* World_cbuffer;
	UINT World_data_length;

	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
	INT Clients_index = 0, Clients_number = 0;
};
