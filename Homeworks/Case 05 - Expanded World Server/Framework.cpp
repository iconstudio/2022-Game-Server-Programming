#include "stdafx.hpp"
#include "Framework.hpp"
#include "SightController.hpp"

IOCPFramework::IOCPFramework()
	: serverListener(NULL), serverAddress(), szAddress(0), completionPort(NULL)
	, serverKey(100), threadWorkers(THREADS_COUNT)
	, acceptOverlap(), acceptBytes(0), acceptCBuffer(), acceptNewbie(NULL)
	, clientsPool(), orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0), mutexClient()
	, myInstances(1000), myPlayerCharacters(CLIENTS_MAX_NUMBER)
	, myWorldView(WORLD_PX_SZ_H, WORLD_PX_SZ_V, SIGHT_PX_SZ_H, SIGHT_PX_SZ_V)

{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	int index = 0;
	std::generate(clientsPool.begin(), clientsPool.end(), [&]() {
		return std::make_shared<Session>(index++, -1, NULL, *this);
	});
}

IOCPFramework::~IOCPFramework()
{
	CloseHandle(completionPort);
	closesocket(serverListener);
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

	serverListener = CreateSocket();
	if (INVALID_SOCKET == serverListener)
	{
		ErrorDisplay("Init → WSASocket()");
		return;
	}

	szAddress = sizeof(serverAddress);
	ZeroMemory(&serverAddress, szAddress);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(serverListener, (SOCKADDR*)(&serverAddress), szAddress))
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

	auto apply = CreateIoCompletionPort((HANDLE)(serverListener)
		, completionPort, serverKey, 0);
	if (NULL == apply)
	{
		ErrorDisplay("CreateIoCompletionPort(serverListener)");
		return;
	}
}

void IOCPFramework::Start()
{
	BOOL option = TRUE;
	if (SOCKET_ERROR == setsockopt(serverListener, SOL_SOCKET, SO_REUSEADDR
		, reinterpret_cast<char*>(&option), sizeof(option)))
	{
		ErrorDisplay("Start → setsockopt(serverListener)");
		return;
	}

	if (SOCKET_ERROR == listen(serverListener, CLIENTS_MAX_NUMBER))
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
	auto result = AcceptEx(serverListener, acceptNewbie, acceptCBuffer
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
		//std::unique_lock barrier(mutexClient);

		auto key = MakeNewbieID();
		auto newbie = acceptNewbie.load();
		auto& session = SeekNewbieSession();
		if (!session)
		{
			std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
			closesocket(newbie);
			return;
		}

		auto index = session->Index;
		auto io = CreateIoCompletionPort(HANDLE(newbie), completionPort, key, 0);
		if (NULL == io)
		{
			ErrorDisplay("ProceedAccept → CreateIoCompletionPort()");
			std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
			closesocket(newbie);
		}
		else
		{
			session->SetStatus(SESSION_STATES::CONNECTED);
			session->SetSocket(newbie);
			session->SetID(key);

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

const shared_ptr<Session>& IOCPFramework::SeekNewbieSession() const
{
	auto it = std::find_if(clientsPool.begin(), clientsPool.end()
		, [&](const shared_ptr<Session>& session) {
		return (session->IsDisconnected());
	});

	return *it;
}

inline SOCKET IOCPFramework::CreateSocket() const
{
	return std::move(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED));
}

void IOCPFramework::Accept(const UINT index)
{
	auto& session = GetClient(index);

	auto character = session.myCharacter.load();
	auto& transform = character->myTransform.myPosition;
	session.SetStatus(SESSION_STATES::ACCEPTED);

	BroadcastSignUp(session);
	BroadcastCreateCharacter(session, transform.x, transform.y);

	SendWorldDataTo(session);
	numberClients++;
}

void IOCPFramework::Disconnect(const UINT index)
{
	auto& session = GetClient(index);
	session.Cleanup();

	if (session.IsAccepted())
	{
		std::unique_lock barrier(mutexClient);
		BroadcastSignOut(session);

		numberClients--;
	}
	else if (session.IsConnected())
	{
	}
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto& client = GetClientByID(PID(key));

	if (nullptr == &client)
	{
		std::cout << "No client - key is " << key << ".\n";
		delete overlap;
	}
	else
	{
		auto exoverlap = static_cast<Asynchron*>(overlap);
		auto op = exoverlap->Operation;

		if (0 == bytes)
		{
			ErrorDisplay("ProceedPacket(bytes=0)");
			return;
		}

		switch (op)
		{
			case ASYNC_OPERATIONS::NONE:
			{}
			break;

			case ASYNC_OPERATIONS::RECV:
			{
				client.ProceedReceived(exoverlap, bytes);
			}
			break;

			case ASYNC_OPERATIONS::SEND:
			{
				client.ProceedSent(exoverlap, bytes);
			}
			break;
		}
	}
}

void IOCPFramework::SendWorldDataTo(Session& who)
{
	ForeachClient([&](const Session& other) {
		if (other != who)
		{
			who.SendSignUp(other.myID);

			auto instance = other.myCharacter.load();
			if (instance)
			{
				who.SendCreateCharacter(other.myID, instance->x, instance->y);
			}
		}
	});
}

void IOCPFramework::BroadcastSignUp(Session& who)
{
	ForeachClient([&](Session& other) {
		other.SendSignUp(who.myID);
	});
}

void IOCPFramework::BroadcastSignOut(Session& who)
{
	ForeachClient([&](Session& other) {
		if (other != who)
		{
			other.SendSignOut(who.myID);
		}
	});
}

void IOCPFramework::BroadcastCreateCharacter(Session& who, CHAR cx, CHAR cy)
{
	ForeachClient([&](Session& other) {
		other.SendCreateCharacter(who.myID, cx, cy);
	});
}

void IOCPFramework::BroadcastMoveCharacterFrom(const UINT index, CHAR nx, CHAR ny)
{
	auto& session = GetClient(index);

	ForeachClient([&](Session& other) {
		other.SendMoveCharacter(session.myID, nx, ny);
	});
}

Session& IOCPFramework::GetClient(const UINT index) const
{
	return *clientsPool[index];
}

Session& IOCPFramework::GetClientByID(const PID id) const
{
	auto it = std::find_if(clientsPool.begin(), clientsPool.end(), [&](const shared_ptr<Session>& session) {
		return (id == session->myID);
	});

	return *(*it);
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
}
