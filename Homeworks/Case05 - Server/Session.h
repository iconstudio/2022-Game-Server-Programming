#pragma once
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "GameEntity.hpp"
#include "PlayerCharacter.hpp"

class Session
{
public:
	Session(UINT index, PID id, IOCPFramework& framework);
	virtual ~Session();

	void SetStatus(SESSION_STATES state);
	void SetSocket(SOCKET sock);
	void SetID(const PID id);

	void AddSight(const PID id);
	void RemoveSight(const PID id);

	void AssignSight(const concurrent_set<PID>& view);
	void AssignSight(const std::vector<PID>& view);
	void AssignSight(std::vector<PID>&& view);

	const concurrent_set<PID>& GetSight() const;
	concurrent_set<PID>& GetSight();
	std::vector<PID> GetLocalSight() const;
	
	SESSION_STATES GetStatus() const volatile;
	SESSION_STATES AcquireStatus() const volatile;
	PID GetID() const volatile;
	PID AcquireID() const volatile;

	void ReleaseStatus(SESSION_STATES state);
	void ReleaseID(PID id);

	virtual bool IsConnected() const volatile;
	virtual bool IsDisconnected() const volatile;
	virtual bool IsAccepted() const volatile;
	virtual bool IsPlayer() const volatile;
	virtual bool IsNonPlayer() const volatile;

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
	bool TryMove(WPARAM input);

	void Cleanup();
	void Disconnect();

	const UINT Index;
	string Nickname;
	std::shared_ptr<PlayerCharacter> Instance;

protected:
	void SetRecvBuffer(const WSABUF& buffer);
	void SetRecvBuffer(LPWSABUF buffer);
	void SetRecvBuffer(CHAR* cbuffer, DWORD size);
	void ClearRecvBuffer();

	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);

	atomic<SESSION_STATES> Status;
	atomic<PID> ID;
	atomic<SOCKET> Socket;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFSIZ];
	DWORD recvBytes;

	IOCPFramework& Framework;

	// 家加等 矫具 备开
	shared_ptr<SightSector> mySightSector;
	// 矫具 格废 (侥喊磊父 历厘)
	concurrent_set<PID> myViewList;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
