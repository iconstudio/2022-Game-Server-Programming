#include "stdafx.hpp"
#include "Session.h"
#include "Commons.hpp"
#include "Network.hpp"
#include "Packet.hpp"
#include "Commons.hpp"
#include "IOCP ServerFramework.hpp"

Session::Session(UINT index, PID id, SOCKET sock, IOCPFramework& framework)
	: Index(index), ID(id), Nickname(), Socket(sock), Framework(framework)
	, Status(SESSION_STATES::NONE)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
	, Instance(nullptr)
{
	ClearOverlap(&recvOverlap);

	recvBuffer.buf = recvCBuffer;
	recvBuffer.len = BUFFSIZE;
	ClearRecvBuffer();
}

Session::~Session()
{
	closesocket(Socket);

	if (Instance)
	{
		Instance.reset();
	}
}

void Session::SetStatus(SESSION_STATES state)
{
	Status = state;
}

void Session::SetSocket(SOCKET sock)
{
	Socket = sock;
}

void Session::SetID(const PID id)
{
	ID = id;
}

void Session::Cleanup()
{
	closesocket(Socket);
	SetStatus(SESSION_STATES::NONE);
	SetSocket(NULL);
	SetID(-1);
}

void Session::Disconnect()
{
	Framework.Disconnect(ID);
}

bool Session::IsConnected() const
{
	return SESSION_STATES::CONNECTED == Status;
}

bool Session::IsDisconnected() const
{
	return SESSION_STATES::NONE == Status;
}

bool Session::IsAccepted() const
{
	return SESSION_STATES::ACCEPTED == Status;
}

void Session::ProceedReceived(EXOVERLAPPED* overlap, DWORD byte)
{
	std::cout << "ProceedReceived (" << ID << ")" << "\n";
	auto& wbuffer = recvBuffer;
	auto& cbuffer = wbuffer.buf;

	recvBytes += byte;

	const auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes)
	{
		auto packet = reinterpret_cast<Packet*>(cbuffer); // 클라이언트 → 서버
		auto sz_want = packet->Size;
		auto type = packet->Type;
		auto pid = packet->playerID;

		if (recvBytes < sz_want)
		{
			auto lack = sz_want - recvBytes;
			std::cout << "클라이언트 " << ID << "에게 받아온 정보가 "
				<< lack << " 만큼 모자라서 다시 수신합니다.\n";
			break;
		}

		switch (type)
		{
			case PACKET_TYPES::CS_SIGNIN:
			{
				auto result = reinterpret_cast<CSPacketSignIn*>(cbuffer);

				if (IsAccepted()) // 잘못된 메시지 받음. 연결 종료.
				{
					Disconnect();
				}
				else // 클라이언트 수용.
				{
					strcpy_s(Nickname, result->Nickname);
					std::cout << ID << "'s Nickname: " << Nickname << ".\n";

					Instance = std::make_shared<PlayerCharacter>(3, 3);

					Framework.RegisterNewbie(Index);
				}
			}
			break;

			case PACKET_TYPES::CS_SIGNOUT:
			{
				if (pid == ID && IsAccepted())
				{
					Disconnect();
					return;
				}
				else // 잘못된 메시지 받음.
				{
				}
			}
			break;

			case PACKET_TYPES::CS_KEY:
			{
				auto result = reinterpret_cast<CSPacketKeyInput*>(cbuffer);

				if (pid == ID && Instance)
				{
					auto key = result->Key;
					bool moved = TryMove(key);
					auto px = Instance->x;
					auto py = Instance->y;

					if (!moved)
					{
						std::cout << "플레이어 " << ID << " - 움직이지 않음.\n";
					}
					else
					{
						std::cout << "플레이어 " << ID
							<< " - 위치: ("
							<< px * CELL_W << ", "
							<< py * CELL_H
							<< ")\n";
					}

					if (moved)
					{
						Framework.BroadcastMoveCharacterFrom(Index, px, py);
					}
				}
				else // 잘못된 메시지 받음.
				{
				}
			}
			break;

			default:
			{
				ClearRecvBuffer();
				ClearOverlap(overlap); // recvOverlap
				ErrorDisplay("ProceedReceived: 잘못된 패킷 받음");

				Disconnect();
				return;
			}
			break;
		}

		recvBytes -= sz_want;
		if (0 < recvBytes)
		{
			MoveStream(cbuffer, sz_want, BUFFSIZE);
		}
	}

	// 아무거나 다 받는다.
	int result = RecvStream(recvBytes);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("ProceedReceived → RecvStream()");
			Disconnect();
			return;
		}
	}
}

void Session::ProceedSent(EXOVERLAPPED* overlap, DWORD byte)
{
	if (0 == byte)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("ProceedSent()");
			Disconnect();
		}
	}

	std::cout << "ProceedSent (" << ID << ")" << "\n";
	auto& sz_send = overlap->sendSize;
	auto& tr_send = overlap->sendSzWant;

	delete overlap;
	//ClearOverlap(overlap);
}

int Session::RecvStream(DWORD size, DWORD begin_bytes)
{
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = size - begin_bytes;

	return Recv(0);
}

int Session::RecvStream(DWORD begin_bytes)
{
	return RecvStream(BUFFSIZE, begin_bytes);
}

void Session::SendSignUp(PID nid)
{
	std::cout << "SendSignUp(" << nid << ")\n";
	auto result = SendPacket<SCPacketSignUp>(nid
		, Framework.GetClientsNumber(), CLIENTS_MAX_NUMBER);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendSignUp()");
			Disconnect();
			return;
		}
	}
}

void Session::SendCreatePlayer(PID id, int cx, int cy)
{
	std::cout << "SendCreatePlayer(" << id << ")\n";
	auto result = SendPacket<SCPacketCreatePlayer>(id, cx, cy);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendCreatePlayer()");
			Disconnect();
			return;
		}
	}
}

void Session::SendAppearEntity(PID cid, int cx, int cy)
{

}

void Session::SendDisppearEntity(PID cid)
{

}

void Session::SendMoveEntity(PID id, int nx, int ny)
{
	std::cout << "SendMoveEntity(" << id << ")\n";
	auto result = SendPacket<SCPacketMoveCharacter>(id, nx, ny);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendMoveEntity()");
			Disconnect();
			return;
		}
	}
}

void Session::SendSignOut(PID rid)
{
	std::cout << "SendSignOut(" << rid << ")\n";
	auto result = SendPacket<SCPacketSignOut>(rid, Framework.GetClientsNumber());
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendSignOut()");
			Disconnect();
			return;
		}
	}
}

void Session::ClearRecvBuffer()
{
	recvBytes = 0;
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));
}

void Session::SetRecvBuffer(const WSABUF& buffer)
{
	recvBuffer = buffer;
}

void Session::SetRecvBuffer(LPWSABUF buffer)
{
	SetRecvBuffer(*buffer);
}

void Session::SetRecvBuffer(CHAR* cbuffer, DWORD size)
{
	recvBuffer.buf = cbuffer;
	recvBuffer.len = size;
}

int Session::Recv(DWORD flags)
{
	if (!recvBuffer.buf) return 0;

	return WSARecv(Socket, &recvBuffer, 1, 0, &flags, &recvOverlap, NULL);
}

int Session::Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap)
{
	if (!datas || !overlap) return 0;

	return WSASend(Socket, datas, count, NULL, 0, overlap, NULL);
}

template<typename PACKET, typename ...Ty>
	requires std::is_base_of_v<Packet, PACKET>
int Session::SendPacket(Ty&&... args)
{
	auto packet = new PACKET{ std::forward<Ty>(args)... };

	auto wbuffer = new WSABUF{};
	wbuffer->buf = reinterpret_cast<char*>(packet);
	wbuffer->len = packet->Size;

	auto overlap = new EXOVERLAPPED{ OVERLAP_OPS::SEND };
	overlap->Type = packet->Type;
	overlap->SetSendBuffer(wbuffer);

	return Send(wbuffer, 1, overlap);
}

void Session::MoveStream(CHAR*& buffer, DWORD position, DWORD max_size)
{
	MoveMemory(buffer, buffer + position, max_size - position);
	ZeroMemory(buffer + max_size - position, position);
}

bool Session::TryMove(WPARAM input)
{
	bool moved = false;
	switch (input)
	{
		case VK_LEFT:
		{
			moved = Instance->TryMoveLT();
		}
		break;

		case VK_RIGHT:
		{
			moved = Instance->TryMoveRT();
		}
		break;

		case VK_UP:
		{
			moved = Instance->TryMoveUP();
		}
		break;

		case VK_DOWN:
		{
			moved = Instance->TryMoveDW();
		}
		break;

		default:
		break;
	}

	return moved;
}
