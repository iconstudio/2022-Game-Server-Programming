#pragma once
#include "stdafx.h"

class Session
{
public:
	Session(PID id, SOCKET sock, IOCPFramework& framework);
	~Session();

	void ClearRecvBuffer();

	template<typename PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, PACKET>
	int SendPacket(Ty... value);
	int Send(Packet* packet);
	int Send(LPWSABUF buffer, const UINT count, LPWSAOVERLAPPED, DWORD begin_bytes = 0);

	void ReceiveStartPosition(DWORD begin_bytes = 0);

	bool ProceedPacket(EXOVERLAPPED* overlap, DWORD byte);

	const PID ID;
	const SOCKET Socket;

	PlayerCharacter const* Instance;

private:
	WSAOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	char recvCBuffer[BUFSIZ];

	EXOVERLAPPED* overlapSendSignUp;
	EXOVERLAPPED* overlapSendCreateChar;
	EXOVERLAPPED* overlapSendMoveChar;
	EXOVERLAPPED* overlapSendSignOut;

	IOCPFramework& Framework;
};
