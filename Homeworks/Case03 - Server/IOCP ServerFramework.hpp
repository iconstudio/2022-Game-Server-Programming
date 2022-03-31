#pragma once
#include "stdafx.h"
#include "Session.h"

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();
	void Accept();
	bool Update();

	tuple<Session*, PID> CreateAndAssignClient(SOCKET nsocket);

	Session* GetClient(PID id);
	Session* GetClientByIndex(UINT index);
	UINT GetClientsNumber() const;

	void RemoveClient(PID rid);
	void RemoveSession(const PID id);

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;

	WSAOVERLAPPED overlapAccept;
	DWORD byteListen;
	vector<SOCKET> socketPool;
	concurrency::concurrent_vector<PID> clientsID;
	PID clientOrderID;
	UINT Clients_number;

	WSAOVERLAPPED overlapRecv;
	WSABUF bufferRecv;
	char cbufferRecv[BUFSIZ];
	UINT szRecv, szWantRecv;
};
