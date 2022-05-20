#pragma once
#include "stdafx.hpp"
#include "Asynchron.hpp"
#include "GameEntity.hpp"
#include "PlayerCharacter.hpp"

class Session
{
public:
	Session(UINT index, PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void SetStatus(SESSION_STATES state);
	void SetSocket(SOCKET sock);
	void SetID(const PID id);
	
	SESSION_STATES GetStatus() const volatile;
	SESSION_STATES AcquireStatus() const volatile;
	PID GetID() const volatile;
	PID AcquireID() const volatile;

	void ReleaseStatus(SESSION_STATES state);
	void ReleaseID(PID id);

	bool IsConnected() const volatile;
	bool IsDisconnected() const volatile;
	bool IsAccepted() const volatile;

	void ProceedReceived(Asynchron* overlap, DWORD byte);
	void ProceedSent(Asynchron* overlap, DWORD byte);

	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	bool TryMove(WPARAM input);

	void Cleanup();
	void Disconnect();

	const UINT Index;

	CHAR Nickname[30];
	std::shared_ptr<PlayerCharacter> Instance;

private:
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

	Asynchron* overlapSendSignUp;
	Asynchron* overlapSendCreateChar;
	Asynchron* overlapSendMoveChar;
	Asynchron* overlapSendSignOut;

	IOCPFramework& Framework;

	shared_concurrent_vector<GameEntity> myViewList;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
