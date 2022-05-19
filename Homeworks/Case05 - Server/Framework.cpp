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

SessionPtr IOCPFramework::GetClient(const UINT index) const
{
	return clientsPool[index].load(std::memory_order_relaxed);
}

SessionPtr IOCPFramework::GetClientByID(const PID id) const
{
	return GetClient(myClients.find(id)->second);
}

UINT IOCPFramework::GetClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_relaxed);
}

bool IOCPFramework::ProceedAccept()
{
	const auto number = AcquireClientsNumber();
	if (CLIENTS_MAX_NUMBER <= number)
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		auto newbie = AcquireNewbieSocket();
		auto key = AcquireNewbieID();
		auto session = FindPlaceForNewbie();

		if (session)
		{
			const auto index = session->Index;
			auto io = CreateIoCompletionPort(HANDLE(newbie), completionPort, key, 0);
			if (NULL == io)
			{
				ErrorDisplay("ProceedAccept → CreateIoCompletionPort()");
				std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
				closesocket(newbie);
			}
			else
			{
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
					}
				}
			}

			ReleaseClientsNumber(number);
			ReleaseClient(index, session);
		}
		else
		{
			std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
			closesocket(newbie);
		}
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	// acceptNewbie 소켓의 소유권 내려놓기
	ReleaseNewbieSocket(CreateSocket());

	Listen();

	return true;
}

void IOCPFramework::ConnectFrom(const UINT index)
{
	auto session = AcquireClient(index);
	auto status = session->AcquireStatus();

	if (SESSION_STATES::CONNECTED == status)
	{
		AddClient(session->GetID(), index);
		++numberClients;

		BroadcastSignUp(session);

		InitializeWorldFor(session);

		status = SESSION_STATES::ACCEPTED;
	}

	session->ReleaseStatus(status);
	ReleaseClient(index, session);
}

void IOCPFramework::Disconnect(const PID id)
{
	if (auto session = GetClientByID(id); session)
	{
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

void IOCPFramework::AddClient(const PID id, const UINT place)
{
	myClients.insert({id, place});
}

void IOCPFramework::RemoveClient(const PID rid)
{

}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto client = GetClientByID(PID(key));

	if (!client)
	{
		std::cout << "No client - key is " << key << ".\n";
		delete static_cast<EXOVERLAPPED*>(overlap);
	}
	else
	{
		volatile auto exoverlap = static_cast<EXOVERLAPPED*>(overlap);
		if (0 == bytes)
		{
			ErrorDisplay("ProceedPacket(bytes=0)");
			return;
		}

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

void IOCPFramework::InitializeWorldFor(SessionPtr& who)
{
	for (auto& info : myClients)
	{
		who->SendSignUp(GetClient(info.first)->GetID());
	}
}

void IOCPFramework::BroadcastSignUp(SessionPtr& who)
{
	for (auto& other : myClients)
	{
		auto session = GetClientByID(other.first);
	}

	ForeachClient([&](const SessionPtr& other) {
		other->SendCreatePlayer(who->ID);
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

UINT IOCPFramework::AcquireClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_acquire);
}

shared_ptr<Session> IOCPFramework::AcquireClient(const UINT index) const
{
	return clientsPool[index].load(std::memory_order_acquire);
}

shared_ptr<Session> IOCPFramework::AcquireClient(const shared_atomic<Session>& ptr) const volatile
{
	return ptr.load(std::memory_order_acquire);
}

SOCKET IOCPFramework::AcquireNewbieSocket() const volatile
{
	return acceptNewbie.load(std::memory_order_acquire);
}

PID IOCPFramework::AcquireNewbieID() const volatile
{
	return orderClientIDs.load(std::memory_order_acquire);
}

shared_ptr<Session> IOCPFramework::FindPlaceForNewbie() const
{
	auto it = std::find_if(clientsPool.cbegin(), clientsPool.cend()
		, [&](const shared_atomic<Session>& session) {
		return (session.load(std::memory_order_relaxed)->IsDisconnected());
	});

	return AcquireClient(*it);
}

void IOCPFramework::ReleaseClientsNumber(const UINT number) volatile
{
	numberClients.store(number, std::memory_order_release);
}

void IOCPFramework::ReleaseClient(const UINT home, shared_ptr<Session>& original)
{
	clientsPool[home].store(original, std::memory_order_release);
}

void IOCPFramework::ReleaseNewbieSocket(const SOCKET n_socket) volatile
{
	acceptNewbie.store(n_socket, std::memory_order_release);
}

void IOCPFramework::ReleaseNewbieID(const PID next) volatile
{
	orderClientIDs.store(next, std::memory_order_release);
}

SOCKET&& IOCPFramework::CreateSocket() const volatile
{
	return std::move(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED));
}
