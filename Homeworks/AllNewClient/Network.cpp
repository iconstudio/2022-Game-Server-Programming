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

	mySemaphore = true;
	mySemaphore.notify_one();

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
	return SendPacket(new CSPacketKeyInput(myProfile.myID, key));
}

bool Network::IsPlayer(PID id) const
{
	return PLAYERS_ID_BEGIN <= id;
}

bool Network::IsNonPlayer(PID id) const
{
	return id < PLAYERS_ID_BEGIN;
}

std::optional<Packet*> Network::OnReceive(DWORD bytes)
{
	//unique_barrier flag(mySemaphore, true);

	std::optional<Packet*> result{};

	recvBytes += bytes;

	constexpr auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes) // while
	{
		auto packet = reinterpret_cast<Packet*>(recvCBuffer);
		auto sz_want = packet->Size;

		if (sz_want <= recvBytes)
		{
			const auto type = packet->Type;
			const auto pid = packet->playerID;

			switch (type)
			{
				case PACKET_TYPES::SC_SIGNUP:
				{
					auto rp = reinterpret_cast<SCPacketSignUp*>(recvCBuffer);
					result = new SCPacketSignUp(*rp);

					if (myProfile.myID == PID(-1))
					{
						myProfile.myID = pid;
					}

					// �ڽ��� ���� ���
					RegisterPlayer(pid);

					// ����
					Update();
				}
				break;

				case PACKET_TYPES::SC_SIGNOUT:
				{
					auto rp = reinterpret_cast<SCPacketSignOut*>(recvCBuffer);
					result = new SCPacketSignOut(*rp);

					if (PID(-1) != pid && pid == myProfile.myID)
					{
						ErrorAbort(L"�߸��� ���� ���� ȣ��!", 0);
					}
					else
					{
						RemovePlayer(pid);
					}
				}
				break;

				case PACKET_TYPES::SC_CREATE_PLAYER:
				{
					auto rp = reinterpret_cast<SCPacketCreatePlayer*>(recvCBuffer);
					result = new SCPacketCreatePlayer(*rp);

					// �ٸ� �÷��̾��� ���� ���
					RegisterPlayer(pid);
				}
				break;

				case PACKET_TYPES::SC_APPEAR_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketAppearCharacter*>(recvCBuffer);
					result = new SCPacketAppearCharacter(*rp);
				}
				break;

				case PACKET_TYPES::SC_DISAPPEAR_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketDisppearCharacter*>(recvCBuffer);
					result = new SCPacketDisppearCharacter(*rp);
				}
				break;

				case PACKET_TYPES::SC_MOVE_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketMoveCharacter*>(recvCBuffer);
					result = new SCPacketMoveCharacter(*rp);
				}
				break;
			}

			MoveMemory(recvCBuffer, recvCBuffer + recvBytes, BUFFSZ - recvBytes);
			recvBytes -= sz_want;
		}
		else
		{
			break;
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

std::optional<Packet*> Network::OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes)
{
	//unique_barrier flag(mySemaphore, true);

	std::optional<Packet*> result{};
	const auto my_async = static_cast<Asynchron*>(asynchron);

	// WSABUF�� ���ۿ� sendCBuffer�� �ٸ�!
	if (0 < bytes)
	{
		auto& my_send_sz = my_async->sendSize;
		my_send_sz += bytes;

		const auto packet = reinterpret_cast<Packet*>(my_async->sendBuffer->buf);
		if (packet && my_async->sendSzWant <= my_async->sendSize)
		{
			result = packet;
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

int Network::Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED asynchron)
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

	return Send(sendBuffer, 1, static_cast<WSAOVERLAPPED*>(asynchron));
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
