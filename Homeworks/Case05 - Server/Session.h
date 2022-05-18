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
	void Cleanup();
	void Disconnect();

	bool IsConnected() const;
	bool IsDisconnected() const;
	bool IsAccepted() const;

	void ProceedReceived(EXOVERLAPPED* overlap, DWORD byte);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	void ProceedSent(EXOVERLAPPED* overlap, DWORD byte);
	void SendSignUp(PID nid);
	void SendSignOut(PID rid);

	void SendCreatePlayer(PID id, float cx, float cy);
	void SendAppearEntity(PID cid, float cx, float cy);
	void SendDisppearEntity(PID cid);
	void SendMoveEntity(PID id, float nx, float ny);

	bool TryMove(WPARAM input);

	PID ID;
	const UINT Index;
	SOCKET Socket;

	SESSION_STATES Status;
	CHAR Nickname[30];
	std::shared_ptr<PlayerCharacter> Instance;

private:
	void SetRecvBuffer(const WSABUF& buffer);
	void SetRecvBuffer(LPWSABUF buffer);
	void SetRecvBuffer(CHAR* cbuffer, DWORD size);
	void ClearRecvBuffer();

	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);

	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	int SendPacket(Ty&&... args);

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
