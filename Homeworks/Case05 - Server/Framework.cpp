#include "pch.hpp"
#include "stdafx.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Session.h"
#include "Commons.hpp"
#include "SightManager.hpp"

constexpr USHORT PORT = 6000;

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, serverKey(100)
	, clientsPool(), orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0), mutexClient()
	, threadWorkers(THREADS_COUNT)
	, mySightManager(WORLD_W, WORLD_H, SIGHT_W, SIGHT_H)
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	for (int i = 0; i < PLAYERS_MAX_NUMBER; ++i)
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

	if (SOCKET_ERROR == listen(Listener, PLAYERS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "서버 시작\n";

	acceptNewbie.store(CreateSocket(), std::memory_order_release);

	Listen();
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);

	while (true)
	{
		//
	}

	std::cout << "서버 종료\n";
}

void IOCPFramework::Update(float time_elapsed)
{

}

void IOCPFramework::Communicate()
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
			ProceedAccept();
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

bool IOCPFramework::IsClientsBound(const UINT index) const
{
	return (0 <= index && index < clientsPool.size());
}

void IOCPFramework::Listen()
{
	auto newbie = acceptNewbie.load(std::memory_order_relaxed);
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
	if (IsClientsBound(index))
	{
		return clientsPool[index].load(std::memory_order_relaxed);
	}
	else
	{
		return nullptr;
	}
}

SessionPtr IOCPFramework::GetClientByID(const PID id) const
{
	if (auto it = myClients.find(id); myClients.end() != it)
	{
		return GetClient(it->second);
	}
	else
	{
		return nullptr;
	}
}

UINT IOCPFramework::GetClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_relaxed);
}

void IOCPFramework::ProceedAccept()
{
	auto number = GetClientsNumber();
	if (PLAYERS_MAX_NUMBER <= number)
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
	}
	else
	{
		auto session = FindPlaceForNewbie();
		auto newbie = AcquireNewbieSocket();

		if (session)
		{
			const auto index = session->Index;
			auto key = AcquireNewbieID();

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

				RegisterPlayer(key, session->Index);

				if (SOCKET_ERROR == session->RecvStream())
				{
					if (WSA_IO_PENDING != WSAGetLastError())
					{
						ErrorDisplay("ProceedAccept → RecvStream()");
						std::cout << "클라이언트 " << key << "에서 오류!\n";
						session->Cleanup();
					}
				}
				else
				{
					key++;
				}
			}

			// 클라이언트 ID의 소유권 내려놓기
			ReleaseNewbieID(key);
			// 클라이언트의 소유권 내려놓기
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
}

void IOCPFramework::ConnectFrom(const UINT index)
{
	auto session = AcquireClient(index);
	auto status = session->AcquireStatus();

	if (SESSION_STATES::CONNECTED == status)
	{
		// 클라이언트 ID 부여
		SendSignUp(session, session->GetID());
		// 시야 정보 전송
		InitializeWorldFor(session);

		status = SESSION_STATES::ACCEPTED;
	}

	session->ReleaseStatus(status);
	ReleaseClient(index, session);
}

void IOCPFramework::Disconnect(const PID id)
{
	if (auto session = AcquireClientByID(id); session)
	{
		const auto index = session->Index;

		if (session->IsAccepted())
		{
			// 서버에서 해당 클라이언트를 삭제
			DeregisterPlayer(id);
			// Broadcast: 클라이언트에게 접속 종료를 통지
			for (auto& player : myClients)
			{
				SendSignOut(GetClient(player.second), id);
			}
			// 원래 클라이언트가 있던 세션 청소
			session->Cleanup();
		}
		else if (session->IsConnected())
		{
			// 서버에서 해당 클라이언트를 삭제
			DeregisterPlayer(id);
			// 원래 클라이언트가 있던 세션 청소
			session->Cleanup();
		}

		ReleaseClient(index, session);
	}
}

void IOCPFramework::RegisterPlayer(const PID id, const UINT place)
{
	const auto value = AcquireClientsNumber();
	myClients.insert({ id, place });
	ReleaseClientsNumber(value + 1);
}

void IOCPFramework::DeregisterPlayer(const PID rid)
{
	const auto value = AcquireClientsNumber();
	myClients.unsafe_erase(myClients.find(rid));
	ReleaseClientsNumber(value - 1);
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto client = GetClientByID(PID(key));

	if (!client || !overlap)
	{
		std::cout << "No client - key is " << key << ".\n";
		if (overlap)
		{
			delete overlap;
		}
	}
	else
	{
		volatile auto exoverlap = static_cast<Asynchron*>(overlap);
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
	//for (auto& player : myClients)
	{
		// 기존에 있던 모든 플레이어의 목록을 전달
		//SendPlayerCreate(who, player.first, GetClient(player.second)->Nickname);
	}

	// 시야 목록을 전달
}

template<typename MY_PACKET, typename ...Ty>
	requires std::is_base_of_v<Packet, MY_PACKET>
std::pair<LPWSABUF, Asynchron*> IOCPFramework::CreateTicket(Ty&&... args)
{
	auto packet = new MY_PACKET(std::remove_cvref_t<Ty>(args)...);

	auto wbuffer = new WSABUF{};
	wbuffer->buf = reinterpret_cast<char*>(packet);
	wbuffer->len = packet->Size;

	auto overlap = new Asynchron{ OVERLAP_OPS::SEND };
	overlap->Type = packet->Type;
	overlap->SetSendBuffer(wbuffer);

	return std::make_pair(wbuffer, overlap);
}

int IOCPFramework::SendSignUp(const SessionPtr& target, const PID id)
{
	std::cout << target->GetID() << " → SendSignUp(" << id << ")\n";

	const auto ticket = CreateTicket<SCPacketSignUp>(id);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendPlayerCreate(const SessionPtr& target, const PID who, char* nickname)
{
	return SendPlayerCreate(SessionPtr(target), who, nickname);
}

int IOCPFramework::SendPlayerCreate(SessionPtr&& target, const PID who, char* nickname)
{
	std::cout << target->GetID() << " → SendPlayerCreate(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketCreatePlayer>(who, nickname);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendSignOut(const SessionPtr& target, const PID who)
{
	return SendSignOut(SessionPtr(target), who);
}

int IOCPFramework::SendSignOut(SessionPtr&& target, const PID who)
{
	std::cout << target->GetID() << " → SendSignOut(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketSignOut>(who, GetClientsNumber());
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendAppearEntity(const SessionPtr& target, PID cid, int type, float cx, float cy)
{
	return SendAppearEntity(SessionPtr(target), cid, type, cx, cy);
}

int IOCPFramework::SendAppearEntity(SessionPtr&& target, PID cid, int type, float cx, float cy)
{
	std::cout << target->GetID() << " → SendAppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketAppearCharacter>(cid, type, cx, cy);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendDisppearEntity(const SessionPtr& target, PID cid)
{
	return SendDisppearEntity(SessionPtr(target), cid);
}

int IOCPFramework::SendDisppearEntity(SessionPtr&& target, PID cid)
{
	std::cout << target->GetID() << " → SendDisppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketDisppearCharacter>(cid);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendMoveEntity(const SessionPtr& target, PID cid, float nx, float ny)
{
	return SendMoveEntity(SessionPtr(target), cid, nx, ny);
}

int IOCPFramework::SendMoveEntity(SessionPtr&& target, PID cid, float nx, float ny)
{
	std::cout << target->GetID() << " → SendMoveEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketMoveCharacter>(cid, nx, ny);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

void IOCPFramework::SetClientsNumber(const UINT number) volatile
{
	numberClients.store(number, std::memory_order_relaxed);
}

UINT IOCPFramework::AcquireClientsNumber() const volatile
{
	return numberClients.load(std::memory_order_acquire);
}

shared_ptr<Session> IOCPFramework::AcquireClient(const UINT index) const
{
	return clientsPool[index].load(std::memory_order_acquire);
}

shared_ptr<Session> IOCPFramework::AcquireClientByID(const PID id) const
{
	return clientsPool[myClients.find(id)->second].load(std::memory_order_acquire);
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
