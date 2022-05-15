#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
#include "Session.h"
#include "Asynchron.hpp"
#include "SightController.hpp"

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

	void Accept(const UINT index);
	void Disconnect(const UINT index);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);
	void SendWorldDataTo(SessionPtr& who);
	void BroadcastSignUp(SessionPtr& who);
	void BroadcastSignOut(SessionPtr& who);
	void BroadcastCreateCharacter(SessionPtr& who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacterFrom(const UINT index, CHAR nx, CHAR ny);

	inline SOCKET CreateSocket() const
	{
		return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	template<typename Predicate> void ForeachClient(Predicate predicate);

	SOCKET serverListener;
	SOCKADDR_IN serverAddress;
	INT szAddress;
	HANDLE completionPort;
	const ULONG_PTR serverKey;
	std::vector<std::thread> threadWorkers;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];
	std::atomic<SOCKET> acceptNewbie;

	std::timed_mutex mutexClient;

	std::array<SessionPtr, CLIENTS_MAX_NUMBER> clientsPool;
	std::atomic<PID> orderClientIDs;
	std::atomic<UINT> numberClients;

	SightController myWorldView;
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
