#include "stdafx.h"
#include "Session.h"
#include "Network.hpp"
#include "IOCP ServerFramework.hpp"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Nickname(), Socket(sock), Framework(framework)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&recvOverlap);

	recvBuffer.buf = recvCBuffer;
	recvBuffer.len = BUFFSIZE;
}

Session::~Session()
{
	if (recvCBuffer) delete[] recvCBuffer;

	delete& recvOverlap;
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

void Session::ClearRecvBuffer()
{
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));
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

		switch (type)
		{
			case PACKET_TYPES::CS_SIGNIN:
			{
				if (sz_want <= recvBytes)
				{
					auto result = reinterpret_cast<CSPacketSignIn*>(cbuffer);

					strcpy_s(Nickname, result->Nickname);
					std::cout << ID << "'s Nickname: " << Nickname << ".\n";

					//TODO: 방송
					SendSignUp(ID);
					SendCreateCharacter(ID, 4, 4);

					recvBytes -= sz_want;
					if (0 < recvBytes)
					{
						MoveStream(cbuffer, sz_want, BUFFSIZE);
					}
				}
				else
				{
					auto lack = sz_want - recvBytes;
					std::cout << "CS_SIGNIN: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";
				}
			}
			break;

			case PACKET_TYPES::CS_SIGNOUT:
			{
				if (sz_want <= recvBytes)
				{
					if (pid == ID)
					{
						Framework.Disconnect(ID);
						return;
					}
					else
					{
						recvBytes -= sz_want;
						if (0 < recvBytes)
						{
							MoveStream(cbuffer, sz_want, BUFFSIZE);
						}
					}
				}
				else
				{
					auto lack = sz_want - recvBytes;
					std::cout << "CS_SIGNOUT: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";
				}
			}
			break;

			case PACKET_TYPES::CS_KEY:
			{
				if (sz_want <= recvBytes)
				{
					auto result = reinterpret_cast<CSPacketKeyInput*>(cbuffer);

					if (pid == ID && Instance)
					{
						auto key = result->Key;
						bool moved = TryMove(key);

						if (!moved)
						{
							std::cout << "플레이어 " << ID << " - 움직이지 않음.\n";
						}
						else
						{
							std::cout << "플레이어 " << ID
								<< " - 위치: ("
								<< Instance->x * CELL_W
								<< ", " << Instance->y * CELL_H
								<< ")\n";
						}

						if (moved)
						{
							//TODO: 방송 BroadcastWorld();
							SendMoveCharacter(ID, Instance->x, Instance->y);
						}
					}

					recvBytes -= sz_want;
					if (0 < recvBytes)
					{
						MoveStream(cbuffer, sz_want, BUFFSIZE);
					}
				}
				else
				{
					auto lack = sz_want - recvBytes;
					std::cout << "CS_KEY: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";
				}
			}
			break;

			default:
			{
				ClearRecvBuffer();
				ClearOverlap(overlap); // recvOverlap
				ErrorDisplay("ProceedReceived: 잘못된 패킷 받음");
				return;
			}
			break;
		}
	}

	// 아무거나 다 받는다.
	RecvStream(recvBytes);
}

void Session::ProceedSent(EXOVERLAPPED* overlap, DWORD byte)
{
	std::cout << "ProceedSent (" << ID << ")" << "\n";
	auto& sz_send = overlap->sendSize;
	auto& tr_send = overlap->sendSzWant;

	ClearOverlap(overlap);
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
			return;
		}
	}
}

void Session::SendCreateCharacter(PID id, CHAR cx, CHAR cy)
{
	std::cout << "SendCreateCharacter(" << id << ")\n";
	auto result = SendPacket<SCPacketCreateCharacter>(id, cx, cy);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendSignUp()");
			return;
		}
	}
}

void Session::SendMoveCharacter(PID id, CHAR nx, CHAR ny)
{
	std::cout << "SendMoveCharacter(" << id << ")\n";
	auto result = SendPacket<SCPacketMoveCharacter>(id, nx, ny);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("SendSignUp()");
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
			ErrorDisplay("SendSignUp()");
			return;
		}
	}
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
int Session::SendPacket(Ty... value)
{
	auto packet = new PACKET{ value... };

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
