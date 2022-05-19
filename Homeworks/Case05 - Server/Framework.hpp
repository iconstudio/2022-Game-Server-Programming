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

	SessionPtr GetClient(const UINT index) const;
	SessionPtr GetClientByID(const PID id) const;
	UINT GetClientsNumber() const volatile;

	friend class Session;

private:
	void Listen();
	bool ProceedAccept();

	UINT AcquireClientsNumber() const volatile;
	shared_ptr<Session> AcquireClient(const UINT index) const;
	shared_ptr<Session> AcquireClient(const shared_atomic<Session>& ptr) const volatile;
	SOCKET AcquireNewbieSocket() const volatile;
	PID AcquireNewbieID() const volatile;

	SOCKET&& CreateSocket() const volatile;
	shared_ptr<Session> FindPlaceForNewbie() const;

	void ReleaseClientsNumber(const UINT number) volatile;
	void ReleaseClient(const UINT home, shared_ptr<Session>& original);
	void ReleaseNewbieSocket(const SOCKET n_socket) volatile;
	void ReleaseNewbieID(const PID next) volatile;

	void ConnectFrom(const UINT index);
	void Disconnect(const PID id);

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	// 
	void BroadcastSignUp(SessionPtr& who);
	void BroadcastSignOut(SessionPtr& who);
	void InitializeWorldFor(SessionPtr& who);

	// 


	// 
	/// <summary>
	/// Ŭ���̾�Ʈ���� ���ο� ������ �˸���, ���� �÷��̾� ������ �����ϵ��� ����Ѵ�.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="who">���� ������ �÷��̾��� ����</param>
	int SendSignUp(SessionPtr& target, const SessionPtr& who);
	/// <summary>
	/// Ŭ���̾�Ʈ�� ������ �����Ű��, �ٸ� Ŭ���̾�Ʈ�� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="who">������ �÷��̾��� ����</param>
	int SendSignOut(SessionPtr& target, const SessionPtr& who);
	/// <summary>
	/// �þ� ���� ��ü�� �������� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	/// <param name="type">����</param>
	/// <param name="cx"></param>
	/// <param name="cy"></param>
	int SendAppearEntity(SessionPtr& target, PID cid, int type, float cx, float cy);
	/// <summary>
	/// �þ� ������ ��ü�� ��������� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	int SendDisppearEntity(SessionPtr& target, PID cid);
	/// <summary>
	/// ��ü�� �̵��� �˸���. (�÷��̾� �ڽŵ� ����)
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	/// <param name="nx"></param>
	/// <param name="ny"></param>
	int SendMoveEntity(SessionPtr& target, PID cid, float nx, float ny);

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

	/// <summary>
	/// NPC, Ư�� ��ü, �÷��̾ �����ϴ� �����
	/// </summary>
	std::array<shared_atomic<Session>, ENTITIES_MAX_NUMBER> clientsPool;
	/// <summary>
	/// �÷��̾��� ID -> clientsPool�� ��ȣ (CLIETNS_ORDER_BEGIN���� ����)
	/// </summary>
	concurrent_map<PID, UINT> myClients;
	atomic<UINT> numberClients;
	atomic<PID> orderClientIDs;
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
