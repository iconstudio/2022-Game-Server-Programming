#include "stdafx.h"
#include "Session.h"
#include "IOCP ServerFramework.hpp"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Socket(sock), Framework(framework)
	, recvOverlap(), recvBuffer(), recvCBuffer()
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&recvOverlap);
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));
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
	overlap->Operation = OVERLAP_OPS::SEND;
	overlap->szWantSend = packet->Size;

	auto woverlap = static_cast<WSAOVERLAPPED*>(overlap);
	return Send(wbuffer, 1, woverlap, 0);
}

int Session::Send(LPWSABUF buffer, const UINT count, LPWSAOVERLAPPED overlap, DWORD begin_bytes)
{
	if (!buffer || !overlap) return 0;

	auto result = WSASend(Socket, buffer, count, NULL, 0, overlap, NULL);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			Framework.Disconnect(ID);
			ErrorDisplay("Session ¡æ Send()");
			return 0;
		}
	}

	return result;
}

void Session::BeginPacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto op = overlap->Operation;

	switch (op)
	{
		case OVERLAP_OPS::NONE:
		{}
		break;

		case OVERLAP_OPS::RECV:
		{
			ProceedRecvPacket(overlap, byte);
		}
		break;

		case OVERLAP_OPS::SEND:
		{
			auto& sz_send = overlap->szSend;
			auto& tr_send = overlap->szWantSend;

			ClearOverlap(overlap);
		}
		break;
	}
}

void Session::ProceedRecvPacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto type = overlap->Type;
	auto& buffer = recvCBuffer;
	auto& sz_recv = overlap->szRecv;
	auto& tr_recv = overlap->szWantRecv;

	sz_recv += byte;
	if (tr_recv <= sz_recv)
	{
		ProceedRecvPacket(overlap, sz_recv);

		//sz_recv -= tr_recv;
	}

	ClearRecvBuffer();
	ClearOverlap(overlap); // recvOverlap
	//sz_recv = 0;
	//tr_recv = 0;
}

void Session::ProceedSendPacket(EXOVERLAPPED* overlap, DWORD byte)
{

}
