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

	SOCKET CreateSocket() const;
	pair<PID, Session*> CreateAndAssignClient(SOCKET nsocket);

	Session* GetClient(PID id);
	Session* GetClientByIndex(UINT index);
	UINT GetClientsNumber() const;

	void RemoveClient(PID rid);
	void Disconnect(const PID id);

private:
	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;

	WSAOVERLAPPED overlapAccept;
	DWORD bytesAccept;
	char cbufferAccept[BUFSIZ];

	LPWSAOVERLAPPED portOverlap;
	DWORD portBytes;
	ULONG_PTR portKey;
	const ULONG_PTR serverKey;

	vector<SOCKET> socketPool;
	concurrency::concurrent_vector<PID> clientsID;
	concurrency::concurrent_unordered_map<PID, Session*> Clients;
	PID orderClientIDs;
	UINT numberClients;

	WSAOVERLAPPED recvOverlap;
	WSABUF bufferRecv;
	char cbufferRecv[BUFSIZ];
	UINT szRecv, szWantRecv;
};
