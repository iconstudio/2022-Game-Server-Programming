#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
#include "Session.h"

const UINT THREADS_COUNT = 6;
using SessionPtr = shared_ptr<Session>;

void CALLBACK IOCPWorker();

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();
	void Update();
	friend void CALLBACK IOCPWorker();

	SessionPtr& GetClient(const UINT index);
	SessionPtr& GetClientByID(const PID id);
	UINT GetClientsNumber() const volatile;

	friend class Session;

private:
	void Listen();
	bool ProceedAccept();
	UINT GetAndAcquireClientsNumber() const volatile;

	PID MakeNewbieID();
	SessionPtr SeekNewbieSession() const;
	void ConnectFrom(const UINT index);
	void SendWorldDataTo(SessionPtr& who);
	void Disconnect(const PID who);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	void BroadcastSignUp(SessionPtr& who);
	void BroadcastSignOut(SessionPtr& who);

	/// <summary>
	/// 클라이언트에게 새로운 접속을 알리고, 로컬 플레이어 세션을 생성하도록 명령한다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">새로 접속한 플레이어의 세션</param>
	int SendSignUp(SessionPtr& target, const SessionPtr& who);
	/// <summary>
	/// 클라이언트의 접속을 종료시키고, 다른 클라이언트에 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">나가는 플레이어의 세션</param>
	int SendSignOut(SessionPtr& target, const SessionPtr& who);
	/// <summary>
	/// 시야 내에 개체가 들어왔음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	/// <param name="type"></param>
	/// <param name="cx"></param>
	/// <param name="cy"></param>
	int SendAppearEntity(SessionPtr& target, PID cid, int type, float cx, float cy);
	/// <summary>
	/// 시야 내에서 개체가 사라졌음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	int SendDisppearEntity(SessionPtr& target, PID cid);
	/// <summary>
	/// 개체의 이동을 알린다. (플레이어 자신도 포함)
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	/// <param name="nx"></param>
	/// <param name="ny"></param>
	int SendMoveEntity(SessionPtr& target, PID cid, float nx, float ny);

	SOCKET&& CreateSocket() const;

	template<typename Predicate> void ForeachClient(Predicate predicate);

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;
	const ULONG_PTR serverKey;
	std::vector<std::thread> threadWorkers;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];
	atomic<SOCKET> acceptNewbie;

	std::array<SessionPtr, CLIENTS_MAX_NUMBER> clientsPool;
	concurrent_map<PID, SessionPtr> myClients;
	atomic<PID> orderClientIDs;
	atomic<UINT> numberClients;
	std::timed_mutex mutexClient;
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
