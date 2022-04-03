#pragma once
#include "stdafx.h"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void ClearRecvBuffer();

	void BeginPacket(EXOVERLAPPED* overlap, DWORD byte);
	void ProceedRecvPacket(EXOVERLAPPED* overlap, DWORD byte);
	void ProceedSendPacket(EXOVERLAPPED* overlap, DWORD byte);

	bool ReceiveSignIn(DWORD begin_bytes = 0);
	bool ReceiveSignOut(DWORD begin_bytes = 0);
	bool ReceiveKey(DWORD begin_bytes = 0);

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
	int Recv(LPWSABUF datas, UINT count, DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);

	int RecvPacket(DWORD size, DWORD begin_bytes);
	int RecvPacket(DWORD begin_bytes = 0);
	template<typename PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, PACKET>
	int SendPacket(Ty... value);

	EXOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;
};
