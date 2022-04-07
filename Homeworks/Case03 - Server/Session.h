#pragma once
#include "stdafx.h"
#include "Network.hpp"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void Disconnect();

	void ProceedReceived(EXOVERLAPPED* overlap, DWORD byte);
	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	void ProceedSent(EXOVERLAPPED* overlap, DWORD byte);
	void SendSignUp(PID nid);
	void SendSignOut(PID rid);
	void SendCreateCharacter(PID id, CHAR cx, CHAR cy);
	void SendMoveCharacter(PID id, CHAR nx, CHAR ny);

	bool TryMove(WPARAM input);

	const PID ID;
	const SOCKET Socket;

	CHAR Nickname[30];
	std::shared_ptr<PlayerCharacter> Instance;

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

	EXOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];
	DWORD recvBytes;

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;
};
