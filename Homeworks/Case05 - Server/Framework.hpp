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
	/// <param name="type"></param>
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
	std::concurr
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
