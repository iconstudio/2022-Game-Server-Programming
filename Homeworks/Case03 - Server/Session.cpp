#include "stdafx.h"
#include "Session.h"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Socket(sock), Framework(framework)
	, overlapRecv(), szRecv(0), szWantRecv(0), bufferRecv(), cbufferRecv()
	, recvPacketDescriptor(PACKET_TYPES::NONE, id)
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&overlapRecv);
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(cbufferRecv, sizeof(cbufferRecv));
	szRecv = 0;
}

template<typename PACKET, typename ...Ty>
	requires std::is_base_of_v<Packet, PACKET>
int Session::SendPacket(Ty... value)
{
	auto packet = new PACKET{ value };

	return Send(packet);
}

int Session::Send(Packet* packet)
{
	auto wbuffer = new WSABUF{};
	wbuffer->buf = reinterpret_cast<char*>(packet);
	wbuffer->len = packet->Size;

	auto overlap = new EXOVERLAPPED{};
	overlap->Socket = Socket;
	overlap->Operation = OVERLAP_OPS::SEND;
	overlap->szWantSend = packet->Size;

	auto woverlap = static_cast<WSAOVERLAPPED*>(overlap);
	return Send(wbuffer, 1, woverlap, 0);
}

int Session::Send(LPWSABUF buffer, const UINT count, LPWSAOVERLAPPED overlap, DWORD begin_bytes)
{
	if (!buffer || !overlap) return 0;

	return WSASend(Socket, buffer, count, NULL, 0, overlap, NULL);
}
