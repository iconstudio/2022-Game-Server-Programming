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
	void AddPlayerSpace(Player* instance);
	UINT GetClientsNumber() const;

	Session* GetClient(INT fid);
	Session* GetClient(LPWSAOVERLAPPED overlap);

	void RemoveClient(INT nid);
	void RemoveClient(LPWSAOVERLAPPED overlap);
	void RemoveSession(const INT id);

	SOCKET Socket;

	friend void CallbackWorld(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

private:
	void AcceptSession();
	void BroadcastWorld();
	void SendWorld(Session* session, DWORD send_bytes = 0);
	void ProceedWorld(Session* session, DWORD send_bytes);

	SOCKADDR_IN Address;
	INT sz_Address;
	WSAOVERLAPPED Overlap;
	WSABUF Buffer;
	CHAR CBuffer[BUFFSIZE];
	ULONG Size_send;

	INT Clients_index = 0;
	vector<WSABUF> World;
	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> OverlapClients;
};
