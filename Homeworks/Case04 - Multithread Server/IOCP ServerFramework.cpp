#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Network.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, serverKey(100)
	, clientsPool(), orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0), mutexClient()
	, threadWorkers(THREADS_COUNT)
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	for (int i = 0; i < CLIENTS_MAX_NUMBER; ++i)
	{
		auto& empty = clientsPool.at(i);
		empty = std::make_shared<Session>(i, -1, NULL, *this);
	}
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
	Listen();

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

	std::for_each(threadWorkers.begin(), threadWorkers.end(), [](std::thread& th) {
		th.join();
	});

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

void IOCPFramework::Listen()
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
	if (CLIENTS_MAX_NUMBER <= GetClientsNumber())
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		std::unique_lock barrier(mutexClient);

		auto key = MakeNewbieID();
		auto session = SeekNewbieSession();
		if (!session)
		{
			std::cout << "클라이언트 " << acceptNewbie << "가 접속에 실패했습니다.\n";
			closesocket(acceptNewbie);
			return;
		}

		auto index = session->Index;
		auto io = CreateIoCompletionPort(HANDLE(acceptNewbie), completionPort, key, 0);
		if (NULL == io)
		{
			ErrorDisplay("ProceedAccept → CreateIoCompletionPort()");
			std::cout << "클라이언트 " << acceptNewbie << "가 접속에 실패했습니다.\n";
			closesocket(acceptNewbie);
		}
		else
		{
			session->SetSocket(acceptNewbie);
			session->SetID(key);
			session->SetStatus(SESSION_STATES::CONNECTED);
			session->RecvStream();
		}

		acceptNewbie = CreateSocket();
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	Listen();
}

PID IOCPFramework::MakeNewbieID()
{
	return orderClientIDs++;
}

SessionPtr IOCPFramework::SeekNewbieSession()
{


	return SessionPtr();
}

bool IOCPFramework::RegisterNewbie(const UINT index)
{
	auto& session = GetClient(index);

	BroadcastSignUp(index);

	BroadcastCreateCharacter(index, session->Instance->x, session->Instance->y);

	SendWorldDataTo(index);

	return false;
}

bool IOCPFramework::RegisterNewbie(SessionPtr& session)
{
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

	return false;
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto client = GetClientByID(PID(key));

	if (!client)
	{
		std::cout << "No client - key is " << key << ".\n";
		delete overlap;
	}
	else
	{
		auto exoverlap = static_cast<EXOVERLAPPED*>(overlap);
		auto op = exoverlap->Operation;

		if (0 == bytes)
		{
			ErrorDisplay("ProceedPacket(bytes=0)");
			return;
		}

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

void IOCPFramework::SendWorldDataTo(SessionPtr& session)
{
	ForeachClient([&](const SessionPtr& other) {
		if (other != session)
		{
			session->SendSignUp(other->ID);

			auto instance = other->Instance;
			if (instance)
			{
				session->SendCreateCharacter(other->ID, instance->x, instance->y);
			}
		}
	});
}

void IOCPFramework::BroadcastSignUp(const UINT index)
{
	std::unique_lock barrier(mutexClient, std::try_to_lock);

	ForeachClient([&](const SessionPtr& other) {
		if (other->Index != index)
		{
			other->SendSignUp(index);
		}
	});

	RegisterNewbie(index);
}

void IOCPFramework::BroadcastSignOut(const UINT index)
{
	ForeachClient([&](const SessionPtr& other) {
		if (other->Index != index)
		{
			other->SendSignOut(index);
		}
	});
}

void IOCPFramework::BroadcastCreateCharacter(const UINT index, CHAR cx, CHAR cy)
{
	auto session = GetClient(index);

	if (session)
	{
		ForeachClient([&](const SessionPtr& other) {
			other->SendCreateCharacter(session->ID, cx, cy);
		});
	}
}

void IOCPFramework::BroadcastMoveCharacter(const UINT index, CHAR nx, CHAR ny)
{
	auto session = GetClient(index);

	if (session)
	{
		ForeachClient([&](const SessionPtr& other) {
			other->SendMoveCharacter(session->ID, nx, ny);
		});
	}
}

SessionPtr& IOCPFramework::GetClient(const UINT index)
{
	return clientsPool[index];
}

SessionPtr& IOCPFramework::GetClientByID(const PID id)
{
	auto it = std::find_if(clientsPool.begin(), clientsPool.end(), [&](SessionPtr& session) {
		return (id == session->ID);
	});

	return *it;
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
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
