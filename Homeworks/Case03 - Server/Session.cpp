#include "stdafx.h"
#include "Session.h"
#include "Network.hpp"
#include "IOCP ServerFramework.hpp"

Session::Session(PID id, SOCKET sock, IOCPFramework& framework)
	: ID(id), Nickname(), Socket(sock), Framework(framework)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer()
	, Instance(nullptr)
{
	ClearRecvBuffer();
	ClearOverlap(&recvOverlap);

	recvBuffer.buf = recvCBuffer;
	recvBuffer.len = BUFFSIZE;
}

Session::~Session()
{
	if (recvCBuffer) delete recvCBuffer;

	delete &recvOverlap;
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

void Session::RoutePacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto op = overlap->Operation;

	switch (op)
	{
		case OVERLAP_OPS::NONE:
		{}
		break;

		case OVERLAP_OPS::RECV:
		{
			ProceedReceived(overlap, byte);
		}
		break;

		case OVERLAP_OPS::SEND:
		{
			ProceedSent(overlap, byte);
		}
		break;
	}
}

void Session::ProceedReceived(EXOVERLAPPED* overlap, DWORD byte)
{
	auto& wbuffer = overlap->recvBuffer; // 세션의 recvBuffer
	auto& cbuffer = wbuffer->buf;
	auto& sz_recv = overlap->recvSize;

	sz_recv += byte;

	const auto sz_min = sizeof(Packet);
	while (sz_min <= sz_recv)
	{
		auto packet = reinterpret_cast<Packet*>(cbuffer); // 클라이언트 → 서버
		auto sz_want = packet->Size;
		auto type = packet->Type;
		auto pid = packet->playerID;

		switch (type)
		{
			case PACKET_TYPES::CS_SIGNIN:
			{
				if (sz_want <= sz_recv)
				{
					auto result = reinterpret_cast<CSPacketSignIn*>(cbuffer);

					if (pid == ID)
					{
						strcpy_s(Nickname, result->Nickname);

						SendSignUp();
					}

					sz_recv -= sz_want;
					if (0 < sz_recv)
					{
						MoveStream(cbuffer, sz_want, BUFFSIZE);
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					std::cout << "CS_SIGNIN: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";

					//RecvStream(sz_recv);
				}
			}
			break;

			case PACKET_TYPES::CS_SIGNOUT:
			{
				if (sz_want <= sz_recv)
				{
					if (pid == ID)
					{
						Framework.Disconnect(ID);
						return;
					}
					else
					{
						sz_recv -= sz_want;
						if (0 < sz_recv)
						{
							MoveStream(cbuffer, sz_want, BUFFSIZE);
						}
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					std::cout << "CS_SIGNOUT: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";

					//RecvStream(sz_recv);
				}
			}
			break;

			case PACKET_TYPES::CS_KEY:
			{
				if (sz_want <= sz_recv)
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
							std::cout << "플레이어 " << ID << " - 위치: ("
								<< Instance->x << ", " << Instance->y << ")\n";
						}

						if (moved)
						{
							//Framework.BroadcastWorld();
						}
					}

					sz_recv -= sz_want;
					if (0 < sz_recv)
					{
						MoveStream(cbuffer, sz_want, BUFFSIZE);
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					std::cout << "CS_KEY: 클라이언트 " << ID << "에게 받아온 정보가 "
						<< lack << " 만큼 모자라서 다시 수신합니다.\n";

					//RecvStream(sz_recv);
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
	RecvStream(sz_recv);
}

void Session::ProceedSent(EXOVERLAPPED* overlap, DWORD byte)
{
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

bool Session::SendSignUp(DWORD begin_bytes)
{
	return false;
}

bool Session::SendCreateCharacter(DWORD begin_bytes)
{
	return false;
}

bool Session::SendMoveCharacter(DWORD begin_bytes)
{
	return false;
}

bool Session::SendSignOut(DWORD begin_bytes)
{
	return false;
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

	auto woverlap = static_cast<LPWSAOVERLAPPED>(overlap);
	return Send(wbuffer, 1, woverlap);
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
