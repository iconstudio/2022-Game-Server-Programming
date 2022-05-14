#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
#include "Asynchron.hpp"
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

	void ProceedReceived(Asynchron* overlap, DWORD byte);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	void ProceedSent(Asynchron* overlap, DWORD byte);
	void SendSignUp(PID nid);
	void SendSignOut(PID rid);
	void SendCreateCharacter(PID id, CHAR cx, CHAR cy);
	void SendMoveCharacter(PID id, CHAR nx, CHAR ny);

	bool TryMove(WPARAM input);

	atomic<PID> ID;
	const UINT Index;
	SOCKET Socket;

	SESSION_STATES Status;
	CHAR Nickname[30];
	shared_ptr<PlayerCharacter> Instance;

private:
	void SetRecvBuffer(const WSABUF& buffer);
	void SetRecvBuffer(LPWSABUF buffer);
	void SetRecvBuffer(CHAR* cbuffer, DWORD size);
	void ClearRecvBuffer();

	int Recv(DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);

	template<typename PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, PACKET>
	int SendPacket(Ty&&... args);

	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];
	DWORD recvBytes;

	Asynchron* overlapSendSignUp;
	Asynchron* overlapSendCreateChar;
	Asynchron* overlapSendMoveChar;
	Asynchron* overlapSendSignOut;

	IOCPFramework& Framework;
};

enum class SESSION_STATES
{
	NONE, CONNECTED, ACCEPTED,
};
