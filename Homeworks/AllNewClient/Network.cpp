#include "pch.hpp"
#include "Network.hpp"
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Commons.hpp"

Network::Network(const ULONG max_clients)
	: clientsMax(max_clients)
{}

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
}

void Network::Update()
{

}

std::optional<Packet>& Network::OnReceive(DWORD bytes)
{
	// // O: 여기에 return 문을 삽입합니다.
}

void Network::OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline SOCKET Network::CreateSocket() const
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}
