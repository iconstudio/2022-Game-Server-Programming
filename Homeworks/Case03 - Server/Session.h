#pragma once
#include "stdafx.h"
#include "Network.hpp"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void RoutePacket(EXOVERLAPPED* overlap, DWORD byte);
	void ProceedReceived(EXOVERLAPPED* overlap, DWORD byte);
	void ProceedSent(EXOVERLAPPED* overlap, DWORD byte);

	int RecvStream(DWORD size, DWORD begin_bytes);
	int RecvStream(DWORD begin_bytes = 0);

	bool SendSignUp(DWORD begin_bytes = 0);
	bool SendCreateCharacter(DWORD begin_bytes = 0);
	bool SendMoveCharacter(DWORD begin_bytes = 0);
	bool SendSignOut(DWORD begin_bytes = 0);

	bool TryMove(WPARAM input);

	const PID ID;
	const SOCKET Socket;

	CHAR Nickname[30];
	PlayerCharacter* Instance;

private:
	void SetRecvBuffer(const WSABUF& buffer);
	void SetRecvBuffer(LPWSABUF buffer);
	void SetRecvBuffer(CHAR* cbuffer, DWORD size);
	void ClearRecvBuffer();

	int Recv(DWORD flags = 0);

	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	template<typename PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, PACKET>
	int SendPacket(Ty... value);

	void MoveStream(CHAR*& buffer, DWORD position, DWORD max_size);

	EXOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;
};
