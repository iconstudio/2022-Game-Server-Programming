#pragma once
#include "stdafx.h"
#include "Network.hpp"
#include "Session.h"

const UINT THREADS_COUNT = 6;
using SessionPtr = std::shared_ptr<Session>;

void IOCPWorker(const UINT index);

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();
	void Update();

	friend void IOCPWorker(const UINT index);

	SessionPtr GetClient(const PID id);
	SessionPtr GetClientByIndex(const UINT index);
	UINT GetClientsNumber() const;

private:
	void Accept();
	void ProceedAccept();
	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	void SendWorldDataTo(Session* session);
	void BroadcastSignUp(const PID who);
	void BroadcastSignOut(const PID who);
	void BroadcastCreateCharacter(const PID who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacter(const PID who, CHAR nx, CHAR ny);

	PID MakeNewClientID();
	SOCKET&& CreateSocket() const;
	bool CreateAndAssignClient(SOCKET nsocket);

	//template<typename Predicate> void ForeachClient(Predicate predicate);
	void RemoveClient(const PID rid);
	void Disconnect(const PID who);

	void AddCandidateSocketToPool(SOCKET&& sock);
	void AddCandidateSocketToPool();

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];
	SOCKET acceptNewbie;

	const ULONG_PTR serverKey;

	std::timed_mutex mutexClient;
	//std::vector<SOCKET> socketPool;
	//std::vector<PID> clientsID;
	//std::unordered_map<PID, SessionPtr> Clients;

	std::array<PID, CLIENTS_MAX_NUMBER> clientsIDPool;
	std::array<SessionPtr, CLIENTS_MAX_NUMBER> clientsPool;
	PID orderClientIDs;
	UINT numberClients;

	std::vector<std::thread> threadWorkers;
};

/*
template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto& comp : Clients)
	{
		auto& session = comp.second;
		predicate(session);
	}
}
*/
