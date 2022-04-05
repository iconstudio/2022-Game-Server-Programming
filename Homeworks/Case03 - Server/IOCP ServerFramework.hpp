#pragma once
#include "stdafx.h"
#include "Network.hpp"
#include "Session.h"

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();

	Session* GetClient(PID id);
	Session* GetClientByIndex(UINT index);
	UINT GetClientsNumber() const;

	void RemoveClient(const PID rid);
	void Disconnect(const PID id);

private:
	void Accept();
	bool Update();
	void ProceedAccept();
	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	SOCKET CreateSocket() const;
	void CreateAndAssignClient(SOCKET nsocket);

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];

	LPWSAOVERLAPPED portOverlap;
	DWORD portBytes;
	ULONG_PTR portKey;
	const ULONG_PTR serverKey;

	std::vector<SOCKET> socketPool;
	std::vector<PID> clientsID;
	concurrency::concurrent_unordered_map<PID, Session*> Clients;
	PID orderClientIDs;
	UINT numberClients;

	WSAOVERLAPPED recvOverlap;
	WSABUF bufferRecv;
	char cbufferRecv[BUFSIZ];
	UINT szRecv, szWantRecv;
};
