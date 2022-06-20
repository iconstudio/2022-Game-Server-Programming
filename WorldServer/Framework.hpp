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
	/// 특정 세션의 시야 목록 갱신. NPC라면 송신은 안 한다.
	/// 현재 시야 목록과 바뀐 시야 목록을 비교해서 전송.
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
	/// 패킷 생성
	/// </summary>
	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	std::pair<LPWSABUF, Asynchron*> CreateTicket(Ty&&... args) const;
	/// <summary>
	/// 클라이언트의 접속을 막는다.
	/// </summary>
	int SendSignInFailed(Session* target, LOGIN_ERROR_TYPES type) const;
	/// <summary>
	/// 클라이언트에게 새로운 접속을 알리고, 로컬 플레이어 세션을 생성하도록 명령한다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">새로 접속한 플레이어의 세션</param>
	/// <param name="nickname">새로 접속한 플레이어의 별명</param>
	int SendPlayerCreate(Session* target, const PID who, char* nickname) const;
	/// <summary>
	/// 클라이언트의 접속을 종료시키고, 다른 클라이언트에 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">나가는 플레이어의 세션</param>
	int SendSignOut(Session* target, const PID who) const;
	/// <summary>
	/// 시야 내에 개체가 들어왔음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	/// <param name="data">객체의 성분</param>
	int SendAppearEntity(Session* target, PID cid, SCPacketAppearEntity data) const;
	/// <summary>
	/// 시야 내에서 개체가 사라졌음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	int SendDisppearEntity(Session* target, PID cid) const;
	/// <summary>
	/// 개체의 이동을 알린다. (플레이어 자신도 포함)
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
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
	/// NPC, 특수 객체, 플레이어를 저장하는 저장소
	/// </summary>
	std::array<shared_atomic<Session>, ENTITIES_MAX_NUMBER> clientsPool;
	/// <summary>
	/// 플레이어의 ID -> clientsPool의 번호 (CLIETNS_ORDER_BEGIN부터 시작)
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
