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

	SessionPtr& GetClient(const UINT index);
	SessionPtr& GetClientByID(const PID id);

	UINT GetClientsNumber() const;
	friend class Session;

private:
	void Listen();
	void ProceedAccept();
	PID MakeNewbieID();
	SessionPtr SeekNewbieSession();
	bool RegisterNewbie(const UINT index);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);
	void SendWorldDataTo(SessionPtr& session);
	void BroadcastSignUp(const UINT index);
	void BroadcastSignOut(const UINT index);
	void BroadcastCreateCharacter(const UINT index, CHAR cx, CHAR cy);
	void BroadcastMoveCharacter(const UINT index, CHAR nx, CHAR ny);

	SOCKET&& CreateSocket() const;

	template<typename Predicate> void ForeachClient(Predicate predicate);
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

	std::array<SessionPtr, CLIENTS_MAX_NUMBER> clientsPool;
	PID orderClientIDs;
	UINT numberClients;

	std::vector<std::thread> threadWorkers;
};

template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto& comp : clientsPool)
	{
		if (comp->IsAccepted())
		{
			auto& session = comp.second;
			predicate(session);
		}
	}
}
