#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Network.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, serverKey(100)
	, socketPool(), clientsID(), Clients(), clientsPool()
	, orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0), mutexClient()
	, threadWorkers(THREADS_COUNT)
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
	setlocale(LC_ALL, "KOREAN");

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

	for (UINT i = 0; i < CLIENTS_MAX_NUMBER; ++i)
	{
		auto sk = CreateSocket();
		if (INVALID_SOCKET == sk)
		{
			ErrorDisplay("IOCPFramework → WSASocket()");
			return;
		}

		BOOL option = TRUE;
		if (SOCKET_ERROR == setsockopt(sk, SOL_SOCKET, SO_REUSEADDR
			, reinterpret_cast<char*>(&option), sizeof(option)))
		{
			ErrorDisplay("Init → setsockopt(socketPool)");
			return;
		}

		socketPool.push_back(sk);
	}
}

void IOCPFramework::Start()
{
	BOOL option = TRUE;
	if (SOCKET_ERROR == setsockopt(Listener, SOL_SOCKET, SO_REUSEADDR
		, reinterpret_cast<char*>(&option), sizeof(option)))
	{
		ErrorDisplay("Start → setsockopt(Listener)");
		return;
	}

	if (SOCKET_ERROR == listen(Listener, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "서버 시작\n";

	acceptNewbie = CreateSocket();
	Accept();

	threadWorkers.emplace_back(::IOCPWorker, 0);
	threadWorkers.emplace_back(::IOCPWorker, 1);
	threadWorkers.emplace_back(::IOCPWorker, 2);
	threadWorkers.emplace_back(::IOCPWorker, 3);
	threadWorkers.emplace_back(::IOCPWorker, 4);
	threadWorkers.emplace_back(::IOCPWorker, 5);

	while (true)
	{
		//
	}
	std::cout << "서버 종료\n";
}

void IOCPFramework::Update()
{
	DWORD portBytes = 0;
	ULONG_PTR portKey = 0;
	WSAOVERLAPPED* portOverlap = nullptr;

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
				Disconnect(PID(key));
			}
			ErrorDisplay("GetQueuedCompletionStatus(2)");
		}
	}
}

void IOCPFramework::Accept()
{
	auto result = AcceptEx(Listener, acceptNewbie, acceptCBuffer
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

void IOCPFramework::ProceedAccept()
{
	if (socketPool.empty())
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		if (!CreateAndAssignClient(acceptNewbie))
		{
			std::cout << "클라이언트 " << acceptNewbie << "가 접속에 실패했습니다.\n";
			closesocket(acceptNewbie);
			//AddCandidateSocketToPool();
		}

		acceptNewbie = CreateSocket();
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

void IOCPFramework::BroadcastSignUp(const PID who)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	ForeachClient([&](const SessionPtr& other) {
		other->SendSignUp(who);
	});
}

void IOCPFramework::BroadcastSignOut(const PID who)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	ForeachClient([&](const SessionPtr& other) {
		if (other->ID != who)
		{
			other->SendSignOut(who);
		}
	});
}

void IOCPFramework::BroadcastCreateCharacter(const PID who, CHAR cx, CHAR cy)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	auto session = GetClient(who);

	if (session)
	{
		ForeachClient([&](const SessionPtr& other) {
			other->SendCreateCharacter(who, cx, cy);
		});
	}
}

void IOCPFramework::BroadcastMoveCharacter(const PID who, CHAR nx, CHAR ny)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	auto session = GetClient(who);

	if (session)
	{
		ForeachClient([&](const SessionPtr& other) {
			other->SendMoveCharacter(who, nx, ny);
		});
	}
}

void IOCPFramework::SendWorldDataTo(Session* target)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	ForeachClient([&](const SessionPtr& other) {
		if (other.get() != target)
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

SessionPtr IOCPFramework::GetClient(const PID id)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	auto it = Clients.find(id);
	if (Clients.end() != it)
	{
		return it->second;
	}
	else
	{
		return SessionPtr(nullptr);
	}
}

SessionPtr IOCPFramework::GetClientByIndex(const UINT index)
{
	//std::unique_lock barrier(mutexClient, std::try_to_lock);

	return Clients[clientsID[index]];
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
}

bool IOCPFramework::CreateAndAssignClient(SOCKET nsocket)
{
	std::unique_lock barrier(mutexClient, std::defer_lock);

	auto period = std::chrono::seconds(1);
	if (barrier.try_lock_for(period))
	{
		auto session = std::make_shared<Session>(orderClientIDs, nsocket, *this);
		if (!session)
		{
			throw std::exception("서버 메모리 부족!");
			return false;
		}

		auto io = CreateIoCompletionPort(HANDLE(nsocket), completionPort, orderClientIDs, 0);
		if (NULL == io)
		{
			ErrorDisplay("CreateAndAssignClient → CreateIoCompletionPort()");
			return false;
		}

		auto nid = orderClientIDs;
		std::cout << "클라이언트 " << nid << " 접속 → 소켓: " << nsocket << "\n";

		orderClientIDs++;

		// IO 진입
		if (SOCKET_ERROR == session->RecvStream())
		{
			std::cout << "클라이언트 " << nid << " 오류 → 0바이트 받음.\n";
			return false;
		};

		clientsID.push_back(nid);
		Clients.insert(std::move(std::make_pair(nid, session)));
		numberClients++;

		return true;
	}
	else
	{
		return false;
	}
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
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	RemoveClient(who);

	BroadcastSignOut(who);

	AddCandidateSocketToPool();
}

SOCKET&& IOCPFramework::CreateSocket() const
{
	return std::move(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED));
}

void IOCPFramework::AddCandidateSocketToPool(SOCKET&& sock)
{
	socketPool.push_back(std::move(sock));
}

void IOCPFramework::AddCandidateSocketToPool()
{
	socketPool.push_back(std::move(CreateSocket()));
}
