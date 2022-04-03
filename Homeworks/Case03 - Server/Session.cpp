#include "stdafx.h"
#include "Session.h"
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

void Session::ProceedRecvPacket(EXOVERLAPPED* overlap, DWORD byte)
{
	auto& wbuffer = overlap->recvBuffer; // ������ recvBuffer
	auto& cbuffer = wbuffer->buf;
	auto& sz_recv = overlap->recvSize;

	sz_recv += byte;

	const auto sz_min = sizeof(Packet);
	if (sz_min <= sz_recv)
	{
		auto packet = reinterpret_cast<Packet*>(cbuffer); // Ŭ���̾�Ʈ �� ����
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
						MoveMemory(cbuffer, cbuffer + sz_want, BUFFSIZE - sz_want);
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					cout << "CS_SIGNIN: Ŭ���̾�Ʈ " << ID << "���� �޾ƿ� ������ "
						<< lack << " ��ŭ ���ڶ� �ٽ� �����մϴ�.\n";

					ReceiveSignIn(sz_recv);
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
							MoveMemory(cbuffer, cbuffer + sz_want, BUFFSIZE - sz_want);
						}
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					cout << "CS_SIGNOUT: Ŭ���̾�Ʈ " << ID << "���� �޾ƿ� ������ "
						<< lack << " ��ŭ ���ڶ� �ٽ� �����մϴ�.\n";

					ReceiveSignOut(sz_recv);
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
							cout << "�÷��̾� " << ID << " - �������� ����.\n";
						}
						else
						{
							cout << "�÷��̾� " << ID << " - ��ġ: ("
								<< Instance->x << ", " << Instance->y << ")\n";
						}

						ReceiveKey(0);
						if (moved)
						{
							//Framework.BroadcastWorld();
						}
					}

					sz_recv -= sz_want;
					if (0 < sz_recv)
					{
						MoveMemory(cbuffer, cbuffer + sz_want, BUFFSIZE - sz_want);
					}
				}
				else
				{
					auto lack = sz_want - sz_recv;
					cout << "CS_KEY: Ŭ���̾�Ʈ " << ID << "���� �޾ƿ� ������ "
						<< lack << " ��ŭ ���ڶ� �ٽ� �����մϴ�.\n";

					ReceiveKey(sz_recv);
				}
			}
			break;

			default:
			{
				ClearRecvBuffer();
				ClearOverlap(overlap); // recvOverlap
				ErrorDisplay("ProceedRecvPacket: �߸��� ��Ŷ ����");
				return;
			}
			break;
		}
	}
	else
	{
	
	}
}

void Session::ProceedSendPacket(EXOVERLAPPED* overlap, DWORD byte)
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

int Session::RecvPacket(DWORD size, DWORD begin_bytes)
{
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = size - begin_bytes;

	return Recv(&recvBuffer, 1);
}

int Session::RecvPacket(DWORD begin_bytes = 0)
{
	return RecvPacket(BUFFSIZE, begin_bytes);
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
	overlap->SetSendBuffer(wbuffer);

	auto woverlap = static_cast<LPWSAOVERLAPPED>(overlap);
	return Send(wbuffer, 1, woverlap);
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
