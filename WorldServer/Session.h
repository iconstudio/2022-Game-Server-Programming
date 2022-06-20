#pragma once
#include "Asynchron.hpp"
#include "GameObject.hpp"
#include "PlayerCharacter.hpp"

class Session
{
public:
	Session(UINT index, PID id, IOCPFramework& framework);
	virtual ~Session();

	void SetStatus(SESSION_STATES state);
	void SetSocket(SOCKET sock);
	void SetID(const PID id);
	void SetAvatar(shared_ptr<GameObject> handle);

	SESSION_STATES AcquireStatus() const volatile;
	PID AcquireID() const volatile;
	shared_ptr<GameObject> AcquireAvatar();

	SESSION_STATES GetStatus() const volatile;
	PID GetID() const volatile;
	shared_ptr<GameObject> GetAvatar();

	void ReleaseStatus(SESSION_STATES state);
	void ReleaseID(PID id);
	void ReleaseAvatar(shared_ptr<GameObject> handle);

	virtual void AddSight(const PID id);
	virtual void RemoveSight(const PID id);
	void AssignSight(const concurrent_set<PID>& view);
	void AssignSight(const std::unordered_set<PID>& view);
	void AssignSight(const std::vector<PID>& view);
	void AssignSight(std::vector<PID>&& view);
	const concurrent_set<PID>& GetSight() const;
	concurrent_set<PID>& GetSight();
	std::unordered_set<PID> GetLocalSight() const;

	virtual bool IsConnected() const volatile;
	virtual bool IsDisconnected() const volatile;
	virtual bool IsAccepted() const volatile;

	void ProceedReceived(Asynchron* overlap, DWORD byte);
	void ProceedSent(Asynchron* overlap, DWORD byte);

	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	void SetSightArea(const shared_ptr<SightSector>& sector);
	void SetSightArea(shared_ptr<SightSector>&& sector);
	const shared_ptr<SightSector>& GetSightArea() const;
	shared_ptr<SightSector>& GetSightArea();

	// 이동
	void TryMove(MOVE_TYPES dir);
	// 평타 공격
	void TryNormalAttack(MOVE_TYPES dir);

	void Cleanup();
	void Disconnect();

	const UINT Index;
	string myNickname;
	shared_atomic<GameObject> myAvatar;

protected:
	void ClearRecvBuffer();

	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);

	atomic<SESSION_STATES> Status;
	atomic<PID> ID;
	atomic<SOCKET> Socket;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSZ];
	DWORD recvBytes;

	IOCPFramework& myFramework;

	// 소속된 시야 구역
	shared_ptr<SightSector> mySightSector;
	// 시야 목록 (식별자만 저장)
	concurrent_set<PID> myViewList;

	lua_State* myLuaMachine;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
