#include "stdafx.h"
#include "Session.h"
#include "IOCP ServerFramework.hpp"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Socket(sock), Framework(framework)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer()
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&recvOverlap);

	recvBuffer.buf = recvCBuffer;
	recvBuffer.len = BUFFSIZE;
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));
}

int Session::Recv(LPWSABUF datas, UINT count, DWORD flags)
{
	if (!datas) return 0;

	return WSARecv(Socket, datas, count, NULL, &flags, &recvOverlap, NULL);
}

int Session::Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap)
{
	if (!datas || !overlap) return 0;

	return WSASend(Socket, datas, count, NULL, 0, overlap, NULL);
}

int Session::RecvPacket(PACKET_TYPES type, DWORD begin_bytes = 0)
{
	recvOverlap.Type = type;
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = BUFFSIZE;

	return 0;
}

template<typename PACKET, typename ...Ty>
	requires std::is_base_of_v<Packet, PACKET>
int Session::SendPacket(Ty... value)
{
	auto packet = new PACKET{ value };

	auto wbuffer = new WSABUF{};
	wbuffer->buf = reinterpret_cast<char*>(packet);
	wbuffer->len = packet->Size;

	auto overlap = new EXOVERLAPPED{ OVERLAP_OPS::SEND };
	overlap->Type = packet->Type;
	overlap->SetSendBuffer((wbuffer));

	auto woverlap = static_cast<LPWSAOVERLAPPED>(overlap);
	return Send(wbuffer, 1, woverlap);
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
			ProceedSendPacket(overlap, byte);
		}
		break;
	}
}

bool Session::ProceedRecvPacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto type = overlap->Type; // 클라이언트 → 서버
	auto& wbuffer = overlap->recvBuffer; // 세션의 recvBuffer
	auto& cbuffer = wbuffer->buf;
	auto& sz_recv = overlap->recvSize;
	auto& tr_recv = overlap->recvSzWant;
	auto check_bytes = [&]() -> bool { return tr_recv <= sz_recv; };

	sz_recv += byte;

	switch (type)
	{
		case PACKET_TYPES::CS_SIGNIN:
		{
			if (check_bytes())
			{

				sz_recv -= tr_recv;
			}
			else
			{
				ReceiveSignIn();
				return false;
			}
		}
		break;

		case PACKET_TYPES::CS_SIGNOUT:
		{

		}
		break;

		case PACKET_TYPES::CS_KEY:
		{
			if (check_bytes())
			{
				return ProceedRecvPacket(overlap, 0);
			}
			else
			{
				return false;
			}
		}
		break;

		default:
		{
			ClearRecvBuffer();
			ClearOverlap(overlap); // recvOverlap
			ErrorDisplay("ProceedRecvPacket: 잘못된 패킷 받음");
			return false;
		}
		break;
	}
}

bool Session::ProceedSendPacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto& sz_send = overlap->sendSize;
	auto& tr_send = overlap->sendSzWant;

	ClearOverlap(overlap);
}

bool Session::ReceiveSignIn(DWORD begin_bytes)
{
	return false;
}

bool Session::ReceiveSignOut(DWORD begin_bytes)
{
	return false;
}

bool Session::ReceiveKey(DWORD begin_bytes)
{
	return false;
}
