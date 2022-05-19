#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
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
	
	SESSION_STATES GetStatus();
	SESSION_STATES AcquireStatus();
	void ReleaseStatus(SESSION_STATES state);

	bool IsConnected() const volatile;
	bool IsDisconnected() const volatile;
	bool IsAccepted() const volatile;

	void Cleanup();
	void Disconnect();

	void ProceedReceived(EXOVERLAPPED* overlap, DWORD byte);
	void ProceedSent(EXOVERLAPPED* overlap, DWORD byte);

	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);


	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	int SendPacket(Ty&&... args);
	int SendSignUp(PID nid);
	int SendSignOut(PID rid);
	int SendCreatePlayer(PID id);
	int SendAppearEntity(PID cid, int type, float cx, float cy);
	int SendDisppearEntity(PID cid);
	int SendMoveEntity(PID id, float nx, float ny);

	bool TryMove(WPARAM input);

	const UINT Index;

	atomic<SESSION_STATES> Status;
	atomic<PID> ID;
	atomic<SOCKET> Socket;

	CHAR Nickname[30];
	std::shared_ptr<PlayerCharacter> Instance;

private:
	void SetRecvBuffer(const WSABUF& buffer);
	void SetRecvBuffer(LPWSABUF buffer);
	void SetRecvBuffer(CHAR* cbuffer, DWORD size);
	void ClearRecvBuffer();

	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);



	EXOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];
	DWORD recvBytes;

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;

	shared_concurrent_vector<GameEntity> myViewList;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
