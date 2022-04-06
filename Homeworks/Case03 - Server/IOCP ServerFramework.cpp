#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Network.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, portOverlap(), portBytes(0), portKey(0), serverKey(100)
	, socketPool(), clientsID(), Clients()
	, orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0), mutexClient()
{
	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	socketPool.reserve(CLIENTS_MAX_NUMBER);
	clientsID.reserve(CLIENTS_MAX_NUMBER);
}

IOCPFramework::~IOCPFramework()
{
	CloseHandle(completionPort);
	closesocket(Listener);
	WSACleanup();
}

void IOCPFramework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	Listener = CreateSocket();
	if (INVALID_SOCKET == Listener)
	{
		ErrorDisplay("Init → WSASocket()");
		return;
	}

	szAddress = sizeof(Address);
	ZeroMemory(&Address, szAddress);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = htonl(INADDR_ANY);
	Address.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(Listener, (SOCKADDR*)(&Address), szAddress))
	{
		ErrorDisplay("bind()");
		return;
	}

	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort)
	{
		ErrorDisplay("CreateIoCompletionPort(INVALID_HANDLE_VALUE)");
		return;
	}

	auto apply = CreateIoCompletionPort((HANDLE)(Listener)
		, completionPort, serverKey, 0);
	if (NULL == apply)
	{
		ErrorDisplay("CreateIoCompletionPort(Listener)");
		return;
	}

	for (UINT i = 0; i < socketPool.capacity(); ++i)
	{
		auto sk = CreateSocket();
		if (INVALID_SOCKET == sk)
		{
			ErrorDisplay("IOCPFramework → WSASocket()");
			return;
		}

		socketPool.push_back(std::move(sk));
	}
}

void IOCPFramework::Start()
{
	if (SOCKET_ERROR == listen(Listener, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "서버 시작\n";
	Accept();

	while (true)
	{
		Update();
	}
	std::cout << "서버 종료\n";
}

void IOCPFramework::Accept()
{
	auto newbie = socketPool.back();

	auto result = AcceptEx(Listener, newbie, acceptCBuffer
		, 0
		, sizeof(SOCKADDR_IN) + 16
		, szAddress + 16
		, &acceptBytes, &acceptOverlap);

	if (FALSE == result)
	{
		auto error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			ClearOverlap(&acceptOverlap);
			ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
			ErrorDisplay("AcceptEx()");
		}
	}
}

void IOCPFramework::Update()
{
	auto result = GetQueuedCompletionStatus(completionPort, &portBytes, &portKey, &portOverlap, INFINITE);
	auto key = portKey;

	if (TRUE == result)
	{
		auto bytes = portBytes;

		std::cout << "GQCS: " << key << ", Bytes: " << bytes << "\n";
		
		if (serverKey == key) // AcceptEx
		{
			ProceedAccept();
		}
		else // Recv / Send
		{
			if (0 == bytes)
			{
				ErrorDisplay("GetQueuedCompletionStatus(1)");
			}

			ProceedPacket(portOverlap, key, bytes);
		}
	}
	else 
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			if (serverKey != key)
			{
				RemoveClient(key);
			}
			ErrorDisplay("GetQueuedCompletionStatus(2)");
		}
	}
}

void IOCPFramework::ProceedAccept()
{
	auto& newbie = socketPool.back();

	if (socketPool.empty())
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		CreateAndAssignClient(newbie);
		socketPool.pop_back();
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
	if (!socketPool.empty())
	{
		Accept();
	}
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto client = GetClient(PID(key));
	if (!client)
	{
		std::cout << "No client - key is " << key << ".\n";
		delete overlap;
	}
	else
	{
		if (client->Dead)
		{
			delete overlap;
			return;
		}

		auto exoverlap = static_cast<EXOVERLAPPED*>(overlap);
		auto op = exoverlap->Operation;

		switch (op)
		{
			case OVERLAP_OPS::NONE:
			{}
			break;

			case OVERLAP_OPS::RECV:
			{
				client->ProceedReceived(exoverlap, bytes);
			}
			break;

			case OVERLAP_OPS::SEND:
			{
				client->ProceedSent(exoverlap, bytes);
			}
			break;
		}
	}
}

SOCKET IOCPFramework::CreateSocket() const
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void IOCPFramework::CreateAndAssignClient(SOCKET nsocket)
{
	auto session = new Session(orderClientIDs, nsocket, *this);
	if (!session)
	{
		throw std::exception("서버 메모리 부족!");
		return;
	}

	auto io = CreateIoCompletionPort((HANDLE)(nsocket), completionPort, orderClientIDs, 0);
	if (NULL == io)
	{
		ErrorDisplay("CreateAndAssignClient → CreateIoCompletionPort()");
		return;
	}

	auto nid = orderClientIDs;
	std::cout << "클라이언트 " << nid << " 접속 → 소켓: " << nsocket << "\n";

	clientsID.push_back(nid);
	Clients.insert(std::move(std::make_pair(nid, session)));
	orderClientIDs++;
	numberClients++;

	session->RecvStream(); // IO 진입
}

Session* IOCPFramework::GetClient(const PID id)
{
	return Clients[id];
}

Session* IOCPFramework::GetClientByIndex(const UINT index)
{
	return Clients[clientsID[index]];
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
}

void IOCPFramework::BroadcastSignUp(const PID who)
{
	auto session = GetClient(who);

	if (session)
	{
		ForeachClient([&](Session* other) {
			other->SendSignUp(who);
		});
	}
}

void IOCPFramework::BroadcastSignOut(const PID who)
{
	ForeachClient([&](Session* other) {
		if (other->ID != who)
		{
			other->SendSignOut(who);
		}
	});
}

void IOCPFramework::BroadcastCreateCharacter(const PID who, CHAR cx, CHAR cy)
{
	auto session = GetClient(who);

	if (session)
	{
		ForeachClient([&](Session* other) {
			other->SendCreateCharacter(who, cx, cy);
		});
	}
}

void IOCPFramework::BroadcastMoveCharacter(const PID who, CHAR nx, CHAR ny)
{
	auto session = GetClient(who);

	if (session)
	{
		ForeachClient([&](Session* other) {
			other->SendMoveCharacter(who, nx, ny);
		});
	}
}

void IOCPFramework::SendWorldDataTo(Session* target)
{
	ForeachClient([&](Session* other) {
		if (other != target)
		{
			target->SendSignUp(other->ID);

			auto instance = other->Instance;
			if (instance)
			{
				target->SendCreateCharacter(other->ID, instance->x, instance->y);
			}
		}
	});
}

void IOCPFramework::RemoveClient(const PID rid)
{
	auto vit = std::find(clientsID.begin(), clientsID.end(), rid);
	if (clientsID.end() != vit)
	{
		clientsID.erase(vit);
	}

	auto mit = Clients.find(rid);
	if (Clients.end() != mit)
	{
		Clients.erase(mit);
		numberClients--;
	}
}

void IOCPFramework::Disconnect(const PID who)
{
	RemoveClient(who);

	BroadcastSignOut(who);

	socketPool.push_back(std::move(CreateSocket()));
}
