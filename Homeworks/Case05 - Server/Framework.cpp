#include "stdafx.hpp"
#include "Framework.hpp"
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

	acceptNewbie.store(CreateSocket(), std::memory_order_seq_cst);

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

	if (TRUE == result)
	{
		std::cout << "GQCS: " << portKey << ", Bytes: " << portBytes << "\n";

		if (serverKey == portKey) // AcceptEx
		{
			if (!ProceedAccept())
			{
				// acceptNewbie의 소유권 내려놓기
				acceptNewbie.store(CreateSocket(), std::memory_order_release);

				// 다시 시작
				Listen();
			}
		}
		else // Recv / Send
		{
			ProceedPacket(portOverlap, portKey, portBytes);
		}
	}
	else
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			if (serverKey != portKey)
			{
				Disconnect(PID(portKey));
			}
			ErrorDisplay("GetQueuedCompletionStatus(2)");
		}
	}
}

void IOCPFramework::Listen()
{
	auto newbie = acceptNewbie.load(std::memory_order_acq_rel);
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

UINT IOCPFramework::GetClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_relaxed);
}

bool IOCPFramework::ProceedAccept()
{
	const auto number = GetAndAcquireClientsNumber();
	if (CLIENTS_MAX_NUMBER <= number)
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		auto key = MakeNewbieID();
		auto session = SeekNewbieSession();
		auto newbie = acceptNewbie.load(std::memory_order_acquire);

		if (!session)
		{
			std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
			closesocket(newbie);
			return false;
		}

		const auto index = session->Index;
		auto io = CreateIoCompletionPort(HANDLE(newbie), completionPort, key, 0);
		if (NULL == io)
		{
			ErrorDisplay("ProceedAccept → CreateIoCompletionPort()");
			std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
			closesocket(newbie);
			return false;
		}
		else
		{
			// acceptNewbie 소켓의 소유권 내려놓기
			acceptNewbie.store(CreateSocket(), std::memory_order_release);

			session->SetSocket(newbie);
			session->SetID(key);
			session->SetStatus(SESSION_STATES::CONNECTED);

			if (SOCKET_ERROR == session->RecvStream())
			{
				if (WSA_IO_PENDING != WSAGetLastError())
				{
					ErrorDisplay("ProceedAccept → RecvStream()");
					std::cout << "클라이언트 " << key << "에서 오류!\n";
					session->Cleanup();
					return false;
				}
			}
		}
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	Listen();
	return true;
}

PID IOCPFramework::MakeNewbieID()
{
	return orderClientIDs.fetch_add(1, std::memory_order_acq_rel);
}

SessionPtr IOCPFramework::SeekNewbieSession() const
{
	auto it = std::find_if(clientsPool.cbegin(), clientsPool.cend()
		, [&](const SessionPtr& session) {
		return (session->IsDisconnected());
	});

	return *it;
}

void IOCPFramework::ConnectFrom(const UINT index)
{
	auto& session = GetClient(index);
	auto status = session->Status.load(std::memory_order_acquire);

	if (SESSION_STATES::CONNECTED == status)
	{
		BroadcastSignUp(session);

		BroadcastCreateCharacter(session, session->Instance->x, session->Instance->y);

		SendWorldDataTo(session);
		numberClients++;

		session->Status.store(SESSION_STATES::ACCEPTED, std::memory_order_release);
	}
}

void IOCPFramework::Disconnect(const PID who)
{
	std::unique_lock barrier(mutexClient);

	auto vit = std::find_if(clientsPool.begin(), clientsPool.end()
		, [&](SessionPtr& session) {
		return (session->ID == who);
	});

	if (clientsPool.end() != vit)
	{
		auto& session = *vit;

		if (session->IsAccepted())
		{
			BroadcastSignOut(session);

			session->Cleanup();
			numberClients--;
		}
		else if (session->IsConnected())
		{
			session->Cleanup();
		}
	}
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto& client = GetClientByID(PID(key));

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

void IOCPFramework::SendWorldDataTo(SessionPtr& who)
{
	ForeachClient([&](const SessionPtr& other) {
		if (other != who)
		{
			who->SendSignUp(other->ID);

			auto& instance = other->Instance;
			if (instance)
			{
				who->SendCreatePlayer(other->ID, instance->x, instance->y);
			}
		}
	});
}

void IOCPFramework::BroadcastSignUp(SessionPtr& who)
{
	ForeachClient([&](const SessionPtr& other) {
		other->SendSignUp(who->ID);
	});
}

void IOCPFramework::BroadcastSignOut(SessionPtr& who)
{
	ForeachClient([&](const SessionPtr& other) {
		if (other != who)
		{
			other->SendSignOut(who->ID);
		}
	});
}

void IOCPFramework::BroadcastCreateCharacter(SessionPtr& who)
{
	ForeachClient([&](const SessionPtr& other) {
		other->SendCreatePlayer(who->ID, cx, cy);
	});
}

void IOCPFramework::BroadcastMoveCharacterFrom(const UINT index, CHAR nx, CHAR ny)
{
	auto& session = GetClient(index);

	ForeachClient([&](const SessionPtr& other) {
		other->SendMoveEntity(session->ID, nx, ny);
	});
}

UINT IOCPFramework::GetAndAcquireClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_acquire);
}

SOCKET&& IOCPFramework::CreateSocket() const
{
	return std::move(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED));
}
