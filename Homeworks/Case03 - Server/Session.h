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

	const PID ID;
	const SOCKET Socket;

	UINT szRecv, szWantRecv;

	PlayerCharacter const* Instance;

private:
	WSAOVERLAPPED overlapRecv;
	WSABUF bufferRecv;
	char cbufferRecv[BUFSIZ];
	Packet recvPacketDescriptor;

	EXWSAOVERLAPPED* overlapSendSignUp;
	EXWSAOVERLAPPED* overlapSendCreateChar;
	EXWSAOVERLAPPED* overlapSendMoveChar;
	EXWSAOVERLAPPED* overlapSendSignOut;

	const IOCPFramework& Framework;
};
