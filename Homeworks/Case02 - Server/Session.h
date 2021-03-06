#pragma once
#include "stdafx.h"

class ServerFramework;
class PlayerCharacter;

class Session
{
public:
	Session(ServerFramework* nframework, ULONG nid, SOCKET sock);
	~Session();

	void ClearRecvBuffer();
	void ClearOverlap(LPWSAOVERLAPPED overlap);

	int RecvPackets(LPWSABUF datas, UINT count, DWORD flags, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
	int SendPackets(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

	void ReceiveStartPosition(DWORD begin_bytes = 0);
	void ProceedStartPosition(DWORD recv_bytes);

	void ReceiveKeyInput(DWORD begin_bytes = 0);
	void ProceedKeyInput (DWORD recv_bytes);
	bool TryMove(WPARAM input);

	void GenerateWorldData();
	void SendWorld(DWORD send_bytes = 0);
	void ProceedWorld(DWORD send_bytes);

	ULONG ID;
	const SOCKET Socket;
	ULONG Size_recv, Size_send_world;
	LPWSAOVERLAPPED Overlap_recv, Overlap_send_world;

	shared_ptr<PlayerCharacter> Instance;
	ServerFramework* Framework;

private:
	SOCKADDR_IN Address;
	INT sz_Address;
	WSABUF Buffer_recv;
	char CBuffer_recv[BUFFSIZE];

	WSABUF World_blob[2];
	PacketInfo World_desc; // 월드 정보 서술자
	PlayerCharacter *LocalWorld;
};
