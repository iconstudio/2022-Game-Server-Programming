#include "pch.hpp"
#include "Network.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Commons.hpp"

Network::Network(const ULONG max_clients)
	: clientsMax(max_clients), myClients()
	, serverIP(), serverPort(PORT)
	, mySocket(NULL), serverAddress(), serverAddressSize(0)
	, recvOverlap(ASYNC_OPERATIONS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
{
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

void Network::Start()
{
	serverAddressSize = sizeof(serverAddress);
	ZeroMemory(&serverAddress, serverAddressSize);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

	int result = WSAConnect(mySocket, reinterpret_cast<SOCKADDR*>(&serverAddress), serverAddressSize, NULL, NULL, NULL, NULL);

	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"WSAConnect()");
			return;
		}
	}
	
	result = Receive();
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"Receive(0)");
			return;
		}
	}
}

void Network::Update()
{

}

std::optional<Packet> Network::OnReceive(DWORD bytes)
{
	std::optional<Packet> result{};

	recvBytes += bytes;

	constexpr auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes) // while
	{
		auto packet = reinterpret_cast<Packet*>(recvCBuffer);
		auto sz_want = packet->Size;

		if (sz_want <= recvBytes)
		{
			auto type = packet->Type;
			auto pid = packet->playerID;

			switch (type)
			{
				case PACKET_TYPES::SC_SIGNUP:
				{
					auto rp = reinterpret_cast<SCPacketSignUp*>(recvCBuffer);
					result = *rp;
				}
				break;

				case PACKET_TYPES::SC_SIGNOUT:
				{
					auto rp = reinterpret_cast<SCPacketSignOut*>(recvCBuffer);
					result = *rp;
				}
				break;

				case PACKET_TYPES::SC_CREATE_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketCreateCharacter*>(recvCBuffer);
					result = *rp;
				}
				break;

				case PACKET_TYPES::SC_MOVE_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketMoveCharacter*>(recvCBuffer);
					result = *rp;
				}
				break;
			}

			MoveMemory(recvCBuffer, recvCBuffer + recvBytes, BUFFSZ - recvBytes);
			recvBytes -= sz_want;
		}
	}

	Receive(recvBytes);

	return result;
}

void Network::OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes)
{
	// // O: 여기에 return 문을 삽입합니다.
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

inline SOCKET Network::CreateSocket() const
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}
