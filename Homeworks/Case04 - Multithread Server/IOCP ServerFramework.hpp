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
	void RegisterNewbie(const UINT index);

	void Disconnect(const PID who);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);
	void SendWorldDataTo(SessionPtr& who);
	void BroadcastSignUp(SessionPtr& who);
	void BroadcastSignOut(SessionPtr& who);
	void BroadcastCreateCharacter(SessionPtr& who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacterFrom(const UINT index, CHAR nx, CHAR ny);

	SOCKET&& CreateSocket() const;

	template<typename Predicate> void ForeachClient(Predicate predicate);

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

	std::array<SessionPtr, CLIENTS_MAX_NUMBER> clientsPool;
	PID orderClientIDs;
	UINT numberClients;

	std::vector<std::thread> threadWorkers;
};

template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto& session : clientsPool)
	{
		if (session->IsAccepted())
		{
			predicate(session);
		}
	}
}
