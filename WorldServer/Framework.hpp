#pragma once
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "Session.h"
#include "SightManager.hpp"

const UINT THREADS_COUNT = 6;
using SessionPtr = shared_ptr<Session>;

void IOCPWorker();
void AIWorker();
void TimerWorker();

class IOCPFramework
{
public:
	IOCPFramework();
	~IOCPFramework();

	void Awake();
	void Start();
	void Update();
	void Release();

	void BuildSessions();
	void BuildNPCs();
	void BuildThreads();
	void Listen();

	friend void IOCPWorker();
	friend void AIWorker();
	friend void TimerWorker();
	friend class Session;

private:
	void ProceedSignUp();
	void ProceedLoginFailed(SOCKET sock, LOGIN_ERROR_TYPES reason);
	void ProceedSignIn(Session* handle, const CSPacketSignIn* packet);

	void RegisterSight(Session* who);
	void InitializeSight(Session* who);
	/// <summary>
	/// Ư�� ������ �þ� ��� ����. NPC��� �۽��� �� �Ѵ�.
	/// ���� �þ� ��ϰ� �ٲ� �þ� ����� ���ؼ� ����.
	/// </summary>
	void UpdateSight(Session* who);
	void CleanupSight(Session* who);

	SessionPtr GetClient(const UINT index) const;
	SessionPtr GetClientByID(const PID id) const;
	SessionPtr GetClient(const PID) const = delete;
	SessionPtr GetClientByID(const UINT) const = delete;
	UINT GetClientsNumber() const volatile;

	SessionPtr CreateNPC(const UINT index, ENTITY_CATEGORY type, int info_index);

	void Disconnect(const PID id);
	void ConnectFrom(const PID) = delete;
	void Disconnect(const UINT) = delete;
	void RegisterPlayer(const PID id, const UINT place);
	void DeregisterPlayer(const PID id);

	void SetClientsNumber(const UINT number) volatile;
	UINT AcquireClientsNumber() const volatile;
	shared_ptr<Session> AcquireClient(const UINT index) const;
	shared_ptr<Session> AcquireClientByID(const PID id) const;
	shared_ptr<Session> AcquireClient(const shared_atomic<Session>& ptr) const volatile;
	SOCKET AcquireNewbieSocket() const volatile;
	PID AcquireNewbieID() const volatile;

	SOCKET CreateSocket() const volatile;
	shared_ptr<Session> FindPlaceForNewbie() const;

	void ReleaseClientsNumber(const UINT number) volatile;
	void ReleaseClient(const UINT home, shared_ptr<Session>& original);
	void ReleaseNewbieSocket(const SOCKET n_socket) volatile;
	void ReleaseNewbieID(const PID next) volatile;

	/// <summary>
	/// ��Ŷ ����
	/// </summary>
	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	std::pair<LPWSABUF, Asynchron*> CreateTicket(Ty&&... args) const;
	/// <summary>
	/// Ŭ���̾�Ʈ�� ������ ���´�.
	/// </summary>
	int SendSignInFailed(Session* target, LOGIN_ERROR_TYPES type) const;
	/// <summary>
	/// Ŭ���̾�Ʈ���� ���ο� ������ �˸���, ���� �÷��̾� ������ �����ϵ��� ����Ѵ�.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="who">���� ������ �÷��̾��� ����</param>
	/// <param name="nickname">���� ������ �÷��̾��� ����</param>
	int SendPlayerCreate(Session* target, const PID who, char* nickname) const;
	/// <summary>
	/// Ŭ���̾�Ʈ�� ������ �����Ű��, �ٸ� Ŭ���̾�Ʈ�� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="who">������ �÷��̾��� ����</param>
	int SendSignOut(Session* target, const PID who) const;
	/// <summary>
	/// �þ� ���� ��ü�� �������� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	/// <param name="data">��ü�� ����</param>
	int SendAppearEntity(Session* target, PID cid, SCPacketAppearEntity data) const;
	/// <summary>
	/// �þ� ������ ��ü�� ��������� �˸���.
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	int SendDisppearEntity(Session* target, PID cid) const;
	/// <summary>
	/// ��ü�� �̵��� �˸���. (�÷��̾� �ڽŵ� ����)
	/// </summary>
	/// <param name="target">Ŭ���̾�Ʈ�� ����</param>
	/// <param name="cid">NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���</param>
	/// <param name="nx"></param>
	/// <param name="ny"></param>
	int SendMoveEntity(Session* target, PID cid, float nx, float ny, MOVE_TYPES dir) const;

	inline const HANDLE GetCompletionPort() const { return completionPort; }

	SOCKET myListener;
	SOCKADDR_IN myAddress;
	INT szAddress;

	HANDLE completionPort;
	const ULONG_PTR serverKey;

	Asynchron acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFFSZ];
	atomic<SOCKET> acceptNewbie;
	std::array<shared_atomic<Session>, ENTITIES_MAX_NUMBER>::const_iterator acceptBeginPlace;

	/// <summary>
	/// NPC, Ư�� ��ü, �÷��̾ �����ϴ� �����
	/// </summary>
	std::array<shared_atomic<Session>, ENTITIES_MAX_NUMBER> clientsPool;
	/// <summary>
	/// �÷��̾��� ID -> clientsPool�� ��ȣ (CLIETNS_ORDER_BEGIN���� ����)
	/// </summary>
	//concurrent_map<PID, UINT> myClients;
	std::unordered_map<PID, UINT> myClients;
	atomic<UINT> numberClients;
	atomic<PID> orderClientIDs;

	SightManager mySightManager;

	std::vector<std::jthread> threadWorkers;
	std::priority_queue<Timeline> timerQueue;
	std::mutex timerMutex;

};
