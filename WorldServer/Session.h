#pragma once
#include "Asynchron.hpp"
#include "GameObject.hpp"
#include "PlayerCharacter.hpp"

class Session
{
public:
	Session(UINT index, PID id, IOCPFramework& framework);
	virtual ~Session();

	void ProceedReceived(Asynchron* overlap, DWORD byte);
	void ProceedSent(Asynchron* overlap, DWORD byte);
	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);
	void Cleanup();
	void Disconnect();

	void SetStatus(SESSION_STATES state);
	void SetSocket(SOCKET sock);
	void SetID(const PID id);
	//void SetAvatar(shared_ptr<GameObject> handle);

	SESSION_STATES AcquireStatus() const volatile;
	PID AcquireID() const volatile;
	//shared_ptr<GameObject> AcquireAvatar();

	SESSION_STATES GetStatus() const volatile;
	PID GetID() const volatile;
	//shared_ptr<GameObject> GetAvatar();

	void ReleaseStatus(SESSION_STATES state);
	void ReleaseID(PID id);
	//void ReleaseAvatar(shared_ptr<GameObject> handle);

	virtual void AddSight(const PID id);
	virtual void RemoveViewOf(const PID id);
	void AssignSight(const concurrent_set<PID>& view);
	void AssignSight(const std::unordered_set<PID>& view);
	void AssignSight(const std::vector<PID>& view);
	void AssignSight(std::vector<PID>&& view);
	//const concurrent_set<PID>& GetSight() const;
	//concurrent_set<PID>& GetSight();
	const std::unordered_set<PID>& GetSight() const;
	std::unordered_set<PID>& GetSight();
	std::unordered_set<PID> GetLocalSight() const;

	void SetSightArea(const shared_ptr<SightSector>& sector);
	void SetSightArea(shared_ptr<SightSector>&& sector);
	const shared_ptr<SightSector>& GetSightArea() const;
	shared_ptr<SightSector>& GetSightArea();

	virtual bool IsConnected() const volatile;
	virtual bool IsDisconnected() const volatile;
	virtual bool IsAccepted() const volatile;

	void SetPosition(float x, float y);
	void SetPosition(const float(&position)[2]);
	void SetPosition(float_pair position);
	const float* GetPosition() const;
	float* GetPosition();

	void SetBoundingBox(const RECT& box);
	void SetBoundingBox(RECT&& box);
	void SetBoundingBox(long left, long top, long right, long bottom);

	float GetBoundingLeft() const;
	float GetBoundingTop() const;
	float GetBoundingRight() const;
	float GetBoundingBottom() const;

	bool CheckCollision(const Session& other) const;
	bool CheckCollision(const Session* other) const;
	bool CheckCollision(const RECT& other) const;

	// 이동
	virtual void TryMove(MOVE_TYPES dir);
	virtual bool TryMoveLT(float distance);
	virtual bool TryMoveRT(float distance);
	virtual bool TryMoveUP(float distance);
	virtual bool TryMoveDW(float distance);

	// 평타 공격
	void TryNormalAttack(MOVE_TYPES dir);

	const UINT Index;

	SCPacketAppearEntity myInfobox;
	string myNickname;
	atomic<SESSION_STATES> Status;
	atomic<PID> ID;
	atomic<SOCKET> Socket;
	ENTITY_CATEGORY& myCategory;
	ENTITY_TYPES& myType;
	int& myLevel;
	int& myHP, &myMaxHP;
	int& myMP, myMaxMP;
	int& myArmour;
	MOVE_TYPES& myDirection;
	float myPosition[2];
	RECT myBoundingBox;

	lua_State* myLuaMachine;

protected:
	void ClearRecvBuffer();
	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);

	IOCPFramework& myFramework;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSZ];
	DWORD recvBytes;

	// 소속된 시야 구역
	shared_ptr<SightSector> mySightSector;
	// 시야 목록 (식별자만 저장)
	std::unordered_set<PID> myViewList;
	//concurrent_set<PID> myViewList;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
