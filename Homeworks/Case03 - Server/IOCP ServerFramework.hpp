#pragma once
#include "stdafx.h"
#include "Network.hpp"
#include "Session.h"

using SessionPtr = std::shared_ptr<Session>;
class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();

	void SendWorldDataTo(Session* session);
	void BroadcastSignUp(const PID who);
	void BroadcastSignOut(const PID who);
	void BroadcastCreateCharacter(const PID who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacter(const PID who, CHAR nx, CHAR ny);

	SessionPtr GetClient(const PID id);
	SessionPtr GetClientByIndex(const UINT index);
	UINT GetClientsNumber() const;

	void Disconnect(const PID who);

private:
	void Update();
	void Accept();
	void ProceedAccept();
	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	bool CreateAndAssignClient(SOCKET nsocket);

	template<typename Predicate>
	void ForeachClient(Predicate predicate);
	void RemoveClient(const PID rid);

	SOCKET&& CreateSocket() const;
	void AddCandidateSocketToPool(SOCKET&& sock);
	void AddCandidateSocketToPool();

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

	std::timed_mutex mutexClient;
	std::vector<SOCKET> socketPool;
	std::vector<PID> clientsID;
	std::unordered_map<PID, SessionPtr> Clients;
	PID orderClientIDs;
	UINT numberClients;
};

template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto comp : Clients)
	{
		auto& session = comp.second;
		predicate(session);
	}
}
