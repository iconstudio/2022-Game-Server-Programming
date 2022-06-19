#include "pch.hpp"
#include "stdafx.hpp"
#include "Network.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Commons.hpp"

Network::Network(const ULONG max_clients)
	: clientsMax(max_clients), myLocalClients()
	, myStatus(NETWORK_STATES::CLOSED), myProfile()
	, serverIP(), serverPort(PORT), mySemaphore(false)
	, mySocket(NULL), serverAddress(), serverAddressSize(0)
	, recvOverlap(ASYNC_OPERATIONS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
{
	myProfile.myNickname = "Nickname";

	ZeroMemory(recvCBuffer, BUFFSZ);
}

Network::~Network()
{}

void Network::Awake()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorAbort(L"WSAStartup()");
		return;
	}

	mySocket = CreateSocket();
	if (INVALID_SOCKET == mySocket)
	{
		ErrorAbort(L"WSASocket()");
		return;
	}
}

void Network::Start(const char* ip)
{
	serverIP = ip;

	serverAddressSize = sizeof(serverAddress);
	ZeroMemory(&serverAddress, serverAddressSize);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	inet_pton(AF_INET, ip, &serverAddress.sin_addr);

	int result = WSAConnect(mySocket, reinterpret_cast<SOCKADDR*>(&serverAddress), serverAddressSize, NULL, NULL, NULL, NULL);

	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"WSAConnect()");
			return;
		}
	}

	result = SendSignInMsg();
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"SendPacket()");
			return;
		}
	}
}

void Network::Update()
{
	if (SOCKET_ERROR == Receive())
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"Receive(0)");
			return;
		}
	}
}

int Network::SendSignInMsg()
{
	return SendPacket(new CSPacketSignIn(myProfile.myNickname.c_str()));
}

int Network::SendSignOutMsg()
{
	return SendPacket(new CSPacketSignOut(myProfile.myID));
}

int Network::SendKeyMsg(WPARAM key)
{
	return SendPacket(new CSPacketMove(myProfile.myID, key));
}

bool Network::IsPlayer(PID id) const
{
	return PLAYERS_ID_BEGIN <= id;
}

bool Network::IsNonPlayer(PID id) const
{
	return id < PLAYERS_ID_BEGIN;
}

std::vector<Packet*> Network::OnReceive(DWORD bytes)
{
	std::vector<Packet*> result{};
	auto& wbuffer = recvBuffer;
	auto& cbuffer = wbuffer.buf;

	recvBytes += bytes;

	constexpr auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes) // while
	{
		auto packet = reinterpret_cast<Packet*>(cbuffer);
		auto sz_want = packet->Size;

		if (recvBytes < sz_want)
		{
			volatile auto lack = sz_want - recvBytes;

			break;
		}
		else
		{
			const auto type = packet->Type;
			const auto pid = packet->playerID;

			switch (type)
			{
				case PACKET_TYPES::SC_SIGNUP:
				{
					auto rp = reinterpret_cast<SCPacketSignUp*>(cbuffer);
					result.push_back(new SCPacketSignUp(*rp));

					if (myProfile.myID == PID(-1))
					{
						myProfile.myID = pid;
					}

					// 자신의 세션 등록
					RegisterPlayer(pid);
				}
				break;

				case PACKET_TYPES::SC_SIGNOUT:
				{
					auto rp = reinterpret_cast<SCPacketSignOut*>(cbuffer);
					result.push_back(new SCPacketSignOut(*rp));

					if (PID(-1) != pid && pid == myProfile.myID)
					{
						ErrorAbort(L"잘못된 접속 종료 호출!", 0);
					}
					else
					{
						RemovePlayer(pid);
					}
				}
				break;

				case PACKET_TYPES::SC_CREATE_PLAYER:
				{
					auto rp = reinterpret_cast<SCPacketCreatePlayer*>(cbuffer);
					result.push_back(new SCPacketCreatePlayer(*rp));

					// 자신 또는 다른 플레이어의 세션 등록
					RegisterPlayer(pid);
				}
				break;

				case PACKET_TYPES::SC_APPEAR_OBJ:
				{
					auto rp = reinterpret_cast<SCPacketAppearCharacter*>(cbuffer);
					result.push_back(new SCPacketAppearCharacter(*rp));
				}
				break;

				case PACKET_TYPES::SC_DISAPPEAR_OBJ:
				{
					auto rp = reinterpret_cast<SCPacketDisppearCharacter*>(cbuffer);
					result.push_back(new SCPacketDisppearCharacter(*rp));
				}
				break;

				case PACKET_TYPES::SC_MOVE_OBJ:
				{
					auto rp = reinterpret_cast<SCPacketMoveCharacter*>(cbuffer);
					result.push_back(new SCPacketMoveCharacter(*rp));
				}
				break;
			}

			recvBytes -= sz_want;
			if (0 < recvBytes)
			{
				MoveStream(cbuffer, sz_want, BUFSIZ);
			}
		}
	}

	if (SOCKET_ERROR == Receive(recvBytes))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay(L"Receive(recvBytes)");
		}
	}

	return result;
}

std::vector<Packet*> Network::OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes)
{
	std::vector<Packet*> result{};
	const auto my_async = static_cast<Asynchron*>(asynchron);

	// WSABUF의 버퍼와 sendCBuffer가 다름!
	if (0 < bytes)
	{
		auto& my_send_sz = my_async->sendSize;
		my_send_sz += bytes;

		const auto packet = reinterpret_cast<Packet*>(my_async->sendBuffer->buf);
		if (packet && my_async->sendSzWant <= my_async->sendSize)
		{
			result.push_back(packet);
		}
	}

	return result;
}

int Network::Receive(DWORD begin_bytes)
{
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = BUFFSZ - begin_bytes;

	DWORD flags = 0;
	return WSARecv(mySocket, &recvBuffer, 1, 0, &flags, &recvOverlap, CallbackRecv);
}

int Network::Send(LPWSABUF datas, UINT count, Asynchron* asynchron)
{
	if (!datas || !asynchron) return 0;

	return WSASend(mySocket, datas, count, NULL, 0, asynchron, CallbackSend);
}

int Network::SendPacket(Packet* packet)
{
	if (!packet) return 0;

	auto sendBuffer = new WSABUF;
	sendBuffer->buf = reinterpret_cast<char*>(packet);
	sendBuffer->len = ULONG(packet->Size);

	auto asynchron = new Asynchron(ASYNC_OPERATIONS::SEND, packet->Type);
	asynchron->SetSendBuffer(sendBuffer);

	return Send(sendBuffer, 1, asynchron);
}

void Network::MoveStream(CHAR*& buffer, DWORD position, DWORD max_size)
{
	MoveMemory(buffer, (buffer + position), max_size - position);
	ZeroMemory(buffer + max_size - position, position);
}

inline SOCKET Network::CreateSocket() const
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}

inline auto Network::FindPlayer(const PID id) const
{
	return myLocalClients.find(id);
}

inline void Network::RegisterPlayer(const PID id)
{
	myLocalClients.try_emplace(id, make_shared<Session>(id));
}

inline void Network::RemovePlayer(const PID id)
{
	if (auto it = FindPlayer(id); it != myLocalClients.end())
	{
		myLocalClients.erase(it);
	}
}

unique_barrier::unique_barrier(atomic_bool& boolean, bool init_flag)
	: myBoolean(boolean), myFlag(init_flag)
{
	myBoolean.wait(!myFlag);
	myBoolean = myFlag;
}

unique_barrier::~unique_barrier()
{
	myBoolean = !myFlag;
	myBoolean.notify_one();
}
