#pragma once
#include "stdafx.h"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void ClearRecvBuffer();

	int Recv(LPWSABUF datas, UINT count, DWORD flags = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);

	int RecvPacket(PACKET_TYPES type, DWORD begin_bytes = 0);
	int RecvPacket(PACKET_TYPES type, DWORD size, DWORD begin_bytes);
	template<typename PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, PACKET>
	int SendPacket(Ty... value);

	void BeginPacket(EXOVERLAPPED* overlap, DWORD byte);
	bool ProceedRecvPacket(EXOVERLAPPED* overlap, DWORD byte);
	bool ProceedSendPacket(EXOVERLAPPED* overlap, DWORD byte);

	bool ReceiveSignIn(DWORD begin_bytes = 0);
	bool ReceiveSignOut(DWORD begin_bytes = 0);
	bool ReceiveKey(DWORD begin_bytes = 0);

	const PID ID;
	const SOCKET Socket;

	PlayerCharacter const* Instance;

private:
	EXOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFFSIZE];

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;
};
