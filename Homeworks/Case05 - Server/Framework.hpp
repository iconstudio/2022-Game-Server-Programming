#pragma once
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "Session.h"
#include "SightManager.hpp"

const UINT THREADS_COUNT = 6;
using SessionPtr = shared_ptr<Session>;

void IOCPWorker();

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Init();
	void Start();
	void Update(float time_elapsed);
	void Communicate();
	friend void IOCPWorker();

	SessionPtr GetClient(const UINT index) const;
	SessionPtr GetClientByID(const PID id) const;
	UINT GetClientsNumber() const volatile;

	friend class Session;

private:
	bool IsClientsBound(const UINT index) const;

	void Listen();
	void ProceedAccept();

	void ConnectFrom(const UINT index);
	void Disconnect(const PID id);
	void RegisterPlayer(const PID id, const UINT place);
	void DeregisterPlayer(const PID id);

	void SetClientsNumber(const UINT number) volatile;
	UINT AcquireClientsNumber() const volatile;
	shared_ptr<Session> AcquireClient(const UINT index) const;
	shared_ptr<Session> AcquireClientByID(const PID id) const;
	shared_ptr<Session> AcquireClient(const shared_atomic<Session>& ptr) const volatile;
	SOCKET AcquireNewbieSocket() const volatile;
	PID AcquireNewbieID() const volatile;

	SOCKET&& CreateSocket() const volatile;
	shared_ptr<Session> FindPlaceForNewbie() const;

	void ReleaseClientsNumber(const UINT number) volatile;
	void ReleaseClient(const UINT home, shared_ptr<Session>& original);
	void ReleaseNewbieSocket(const SOCKET n_socket) volatile;
	void ReleaseNewbieID(const PID next) volatile;

	void ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes);

	// 
	void InitializeWorldFor(SessionPtr& who);

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

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;
	const ULONG_PTR serverKey;
	std::vector<std::jthread> threadWorkers;

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

	SightManager mySightManager;
};
