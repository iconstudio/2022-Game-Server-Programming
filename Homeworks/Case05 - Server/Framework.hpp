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

	/// <summary>
	/// 클라이언트의 첫 초기화를 위한 정보 전송
	/// </summary>
	/// <param name="who"></param>
	void InitializeWorldFor(SessionPtr& who);
	/// <summary>
	/// 특정 플레이어 주변의 시야 목록 갱신 및 전송
	/// </summary>
	/// <param name="who"></param>
	void UpdateViewOf(SessionPtr& who);

	/// <summary>
	/// 클라이언트에게 패킷 전송
	/// </summary>
	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	std::pair<LPWSABUF, Asynchron*> CreateTicket(Ty&&... args);
	/// <summary>
	/// 새로 들어온 클라이언트에 식별자 부여
	/// </summary>
	/// <param name="target">새로 접속한 플레이어 세션</param>
	/// <param name="id"></param>
	/// <returns></returns>
	int SendSignUp(const SessionPtr& target, const PID id);
	/// <summary>
	/// 클라이언트에게 새로운 접속을 알리고, 로컬 플레이어 세션을 생성하도록 명령한다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">새로 접속한 플레이어의 세션</param>
	/// <param name="nickname">새로 접속한 플레이어의 별명</param>
	int SendPlayerCreate(const SessionPtr& target, const PID who, char* nickname);
	int SendPlayerCreate(SessionPtr&& target, const PID who, char* nickname);
	/// <summary>
	/// 클라이언트의 접속을 종료시키고, 다른 클라이언트에 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="who">나가는 플레이어의 세션</param>
	int SendSignOut(const SessionPtr& target, const PID who);
	int SendSignOut(SessionPtr&& target, const PID who);
	/// <summary>
	/// 시야 내에 개체가 들어왔음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	/// <param name="type">종류</param>
	/// <param name="cx"></param>
	/// <param name="cy"></param>
	int SendAppearEntity(const SessionPtr& target, PID cid, int type, float cx, float cy);
	int SendAppearEntity(SessionPtr&& target, PID cid, int type, float cx, float cy);
	/// <summary>
	/// 시야 내에서 개체가 사라졌음을 알린다.
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	int SendDisppearEntity(const SessionPtr& target, PID cid);
	int SendDisppearEntity(SessionPtr&& target, PID cid);
	/// <summary>
	/// 개체의 이동을 알린다. (플레이어 자신도 포함)
	/// </summary>
	/// <param name="target">클라이언트의 세션</param>
	/// <param name="cid">NPC, 특수 객체, 플레이어의 고유 식별자</param>
	/// <param name="nx"></param>
	/// <param name="ny"></param>
	int SendMoveEntity(const SessionPtr& target, PID cid, float nx, float ny);
	int SendMoveEntity(SessionPtr&& target, PID cid, float nx, float ny);

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
	/// NPC, 특수 객체, 플레이어를 저장하는 저장소
	/// </summary>
	std::array<shared_atomic<Session>, ENTITIES_MAX_NUMBER> clientsPool;
	/// <summary>
	/// 플레이어의 ID -> clientsPool의 번호 (CLIETNS_ORDER_BEGIN부터 시작)
	/// </summary>
	concurrent_map<PID, UINT> myClients;
	atomic<UINT> numberClients;
	atomic<PID> orderClientIDs;
	std::timed_mutex mutexClient;

	SightManager mySightManager;
};
