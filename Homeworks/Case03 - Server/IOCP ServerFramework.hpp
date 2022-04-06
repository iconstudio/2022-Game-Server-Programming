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

	Session* GetClient(const PID id);
	Session* GetClientByIndex(const UINT index);
	UINT GetClientsNumber() const;

	void BroadcastSignUp(const PID who);
	void BroadcastSignOut(const PID who);
	void BroadcastCreateCharacter(const PID who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacter(const PID who, CHAR nx, CHAR ny);

	void SendWorldDataTo(Session* session);
	template<typename Predicate>
	void ForeachClient(Predicate predicate);

	void Disconnect(const PID who);

private:
	void Accept();
	void Update();
	void ProceedAccept();
	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	SOCKET CreateSocket() const;
	void CreateAndAssignClient(SOCKET nsocket);
	void RemoveClient(const PID rid);

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

template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto& comp : Clients)
	{
		auto session = comp.second;
		predicate(session);
	}
}
