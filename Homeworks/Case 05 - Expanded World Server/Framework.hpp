#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
#include "Session.h"
#include "Asynchron.hpp"
#include "SightController.hpp"

const int THREADS_COUNT = 6;

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

	Session& GetClient(const UINT index) const;
	Session& GetClientByID(const PID id) const;
	UINT GetClientsNumber() const;

	friend class Session;

private:
	void Listen();
	void ProceedAccept();

	PID MakeNewbieID();
	const shared_ptr<Session>& SeekNewbieSession() const;
	inline SOCKET CreateSocket() const;

	void Accept(const UINT index);
	void Disconnect(const UINT index);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);
	void SendWorldDataTo(Session& who);
	void BroadcastSignUp(Session& who);
	void BroadcastSignOut(Session& who);
	void BroadcastCreateCharacter(Session& who, CHAR cx, CHAR cy);
	void BroadcastMoveCharacterFrom(const UINT index, CHAR nx, CHAR ny);

	template<typename Predicate> void ForeachClient(Predicate predicate);

	SOCKET serverListener;
	SOCKADDR_IN serverAddress;
	INT szAddress;
	HANDLE completionPort;
	const ULONG_PTR serverKey;
	std::vector<std::jthread> threadWorkers;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];
	std::atomic<SOCKET> acceptNewbie;

	std::timed_mutex mutexClient;

	std::array<shared_ptr<Session>, CLIENTS_MAX_NUMBER> clientsPool;
	std::atomic<PID> orderClientIDs;
	std::atomic<UINT> numberClients;

	shared_atomic_concurrent_vector<GameObject> myInstances;
	shared_atomic_concurrent_vector<GameObject> myPlayerCharacters;

	SightController myWorldView;
};

template<typename Predicate>
inline void IOCPFramework::ForeachClient(Predicate predicate)
{
	for (auto& session : clientsPool)
	{
		if (session->IsAccepted())
		{
			predicate(*session);
		}
	}
}
