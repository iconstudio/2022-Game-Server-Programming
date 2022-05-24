#include "pch.hpp"
#include "stdafx.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Session.h"
#include "PlayingSession.hpp"
#include "Commons.hpp"
#include "SightManager.hpp"
#include "SightSector.hpp"

constexpr USHORT PORT = 6000;

int SightDistance(const XMFLOAT3& pos1, const XMFLOAT3 pos2)
{
	const auto x1 = int(pos1.x);
	const auto y1 = int(pos1.y);
	const auto x2 = int(pos2.x);
	const auto y2 = int(pos2.y);

	return int(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, serverKey(100)
	, clientsPool(), orderClientIDs(CLIENTS_ID_BEGIN), numberClients(0), mutexClient()
	, threadWorkers(THREADS_COUNT)
	, mySightManager(*this, WORLD_W, WORLD_H, SIGHT_W, SIGHT_H)
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	// ������ ���÷��̾� ����
	for (int i = 0; i < NPC_MAX_NUMBER; ++i)
	{
		auto& empty = clientsPool.at(i);
		empty = std::make_shared<Session>(i, -1, *this);
	}

	// ������ �÷��̾� ���� (PLAYERS_MAX_NUMBER)
	for (int j = NPC_MAX_NUMBER; j < ENTITIES_MAX_NUMBER; ++j)
	{
		auto& empty = clientsPool.at(j);

		auto player = std::make_shared<PlayingSession>(j, -1, *this);
		empty = std::static_pointer_cast<Session>(player);
	}

	acceptBeginPlace = clientsPool.cbegin() + CLIENTS_ORDER_BEGIN;
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
		ErrorDisplay("Init �� WSASocket()");
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
		ErrorDisplay("Start �� setsockopt(Listener)");
		return;
	}

	if (SOCKET_ERROR == listen(Listener, PLAYERS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "���� ����\n";

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

	std::cout << "���� ����\n";
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

void IOCPFramework::UpdateSightOf(const UINT index)
{
	auto session = AcquireClient(index);

	// NPC, Ư�� ��ü, �÷��̾��� ���� �ĺ���
	const PID my_id = session->AcquireID();

	// ���� �þ�
	std::vector<PID> viewlist_prev = session->GetLocalSight();

	const auto& character = session->Instance;
	const auto& my_pos = character->GetPosition();
	const auto& my_type = character->myType;

	// 0. �ڱ� ĳ���Ͱ� ���� ������ �����Ѵ�.
	const auto curr_pos = float_pair(my_pos.x, my_pos.y);
	const auto curr_coords = mySightManager.PickCoords(curr_pos);
	auto& curr_sector = mySightManager.At(curr_coords);

	// ������ ������ ������ ȹ��
	curr_sector->Acquire();

	// �ٸ� �޼����� ������ ȹ���� �ȵȴ�
	auto& prev_sector = session->GetSightArea();
	if (nullptr == prev_sector || prev_sector->TryAcquire())
	{
		if (curr_sector != prev_sector)
		{
			// ���� �þ� ������ ����
			if (prev_sector)
			{
				prev_sector->Remove(my_id);
			}
			// �þ� ������ ���
			curr_sector->Add(my_id);
			session->SetSightArea(curr_sector);
		}

		// ���� ������ ������ ������ ��������
		prev_sector->Release();
	}

	// 1. �ڱⰡ ���� ������ ����� ��´�.
	std::vector<PID> viewlist_curr = curr_sector->GetSightList();
	viewlist_curr.reserve(viewlist_curr.capacity() + 50);

	// 2. �þ� �簢���� ��� �������� ã�´�.
	//std::unordered_set<PID> additions;

	constexpr float sgh_w = SIGHT_RAD_W;
	constexpr float sgh_h = SIGHT_RAD_H;

	// 3. �� ������ �þ� ����� ���Ѵ�.

	const auto lu_coords = curr_coords + int_pair{ -sgh_w, -sgh_h };
	if (0 < lu_coords.first && 0 < lu_coords.second)
	{
		const auto& list = mySightManager.At(lu_coords)->GetSightList();
		std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
			viewlist_curr.push_back(ot_id);
		});
		//additions.insert(list.begin(), list.end());
	}

	const auto ru_coords = curr_coords + int_pair{ +sgh_w, -sgh_h };
	if (ru_coords.first < WORLD_CELLS_CNT_H && 0 < ru_coords.second)
	{
		const auto& list = mySightManager.At(ru_coords)->GetSightList();
		std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
			viewlist_curr.push_back(ot_id);
		});
		//additions.insert(list.begin(), list.end());
	}

	const auto ld_coords = curr_coords + int_pair{ -sgh_w, +sgh_h };
	if (0 < ld_coords.first && ld_coords.second < WORLD_CELLS_CNT_V)
	{
		const auto& list = mySightManager.At(ld_coords)->GetSightList();
		std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
			viewlist_curr.push_back(ot_id);
		});
		//additions.insert(list.begin(), list.end());
	}

	const auto rd_coords = curr_coords + int_pair{ +sgh_w, +sgh_h };
	if (rd_coords.first < WORLD_CELLS_CNT_H && rd_coords.second < WORLD_CELLS_CNT_V)
	{
		const auto& list = mySightManager.At(rd_coords)->GetSightList();
		std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
			viewlist_curr.push_back(ot_id);
		});
		//additions.insert(list.begin(), list.end());
	}
	curr_sector->Release();

	// 4. ���ο� �þ� ��� �Ҵ�
	session->AssignSight(viewlist_curr);

	// 5. �þ� ����� �������� ����
	// * ���� ���� ��ü�� Disappear
	// * ���� �ִ� ��ü��, ���ſ��� ������ Move, ������ Appear
	PID cid, pid;
	constexpr int sight_magnitude = SIGHT_CELLS_RAD_H * SIGHT_CELLS_RAD_V;

	for (auto cit = viewlist_curr.cbegin(); viewlist_curr.cend() != cit;)
	{
		cid = *cit;

		auto other = AcquireClientByID(cid);
		if (!other)
		{
			session->RemoveSight(cid);
			cit++;
			continue;
		}

		const bool ot_is_player = other != session && other->IsPlayer();
		const auto& ot_inst = other->Instance;
		const auto& ot_pos = ot_inst->GetPosition();

		if (sight_magnitude < SightDistance(my_pos, ot_pos))
		{
			session->RemoveSight(cid);
			SendDisppearEntity(session, pid);
		}

		const auto pit = std::find(viewlist_prev.cbegin(), viewlist_prev.cend(), cid);

		if (viewlist_prev.cend() == pit)
		{
			// Appear: ���ο� ��ü ���
			session->AddSight(cid);
			SendAppearEntity(session, cid, ot_inst->myType, ot_pos.x, ot_pos.y);

			// ��뵵 ��ü ���
			if (ot_is_player)
			{
				other->AddSight(my_id);
				SendAppearEntity(other, my_id, my_type, my_pos.x, my_pos.y);
			}
		}
		else
		{
			// Move
			SendMoveEntity(session, cid, ot_pos.x, ot_pos.y);
			if (ot_is_player)
			{
				SendMoveEntity(other, my_id, my_pos.x, my_pos.y);
			}

			viewlist_prev.erase(pit);
		}

		ReleaseClient(other->Index, other);
		cit++;
	}

	// Disappear
	for (auto pit = viewlist_prev.cbegin(); viewlist_prev.cend() != pit;
		++pit)
	{
		pid = *pit;

		auto other = GetClientByID(pid);
		const bool ot_is_player = other && other != session && other->IsPlayer();

		session->RemoveSight(pid);
		SendDisppearEntity(session, pid);
		if (ot_is_player)
		{
			other->RemoveSight(my_id);
			SendDisppearEntity(other, my_id);
		}
	}

	session->ReleaseID(my_id);

	ReleaseClient(index, session);
}

SessionPtr IOCPFramework::CreateNPC(const UINT index, ENTITY_TYPES type, int info_index)
{
	return SessionPtr();
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
		std::cout << "�� ������ ���� �� �����ϴ�!\n";
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
				ErrorDisplay("ProceedAccept �� CreateIoCompletionPort()");
				std::cout << "Ŭ���̾�Ʈ " << newbie << "�� ���ӿ� �����߽��ϴ�.\n";
				closesocket(newbie);
			}
			else
			{
				session->SetID(key);
				session->SetSocket(newbie);
				session->SetStatus(SESSION_STATES::CONNECTED);

				RegisterPlayer(key, session->Index);

				if (SOCKET_ERROR == session->RecvStream())
				{
					if (WSA_IO_PENDING != WSAGetLastError())
					{
						ErrorDisplay("ProceedAccept �� RecvStream()");
						std::cout << "Ŭ���̾�Ʈ " << key << "���� ����!\n";
						session->Cleanup();
					}
				}
				else
				{
					key++;
				}
			}

			// Ŭ���̾�Ʈ ID�� ������ ��������
			ReleaseNewbieID(key);
			// Ŭ���̾�Ʈ�� ������ ��������
			ReleaseClient(index, session);
		}
		else
		{
			std::cout << "Ŭ���̾�Ʈ " << newbie << "�� ���ӿ� �����߽��ϴ�.\n";
			closesocket(newbie);
		}
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	// acceptNewbie ������ ������ ��������
	ReleaseNewbieSocket(CreateSocket());

	Listen();
}

void IOCPFramework::ConnectFrom(const UINT index)
{
	auto session = AcquireClient(index);
	auto status = session->AcquireStatus();

	if (SESSION_STATES::CONNECTED == status)
	{
		// Ŭ���̾�Ʈ ID �ο�
		SendSignUp(session, session->GetID());

		// �þ� ���� ����
		InitializeWorldFor(index, session);

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
			// �þ� �������� �ش� Ŭ���̾�Ʈ�� ����
			auto& curr_sector = session->GetSightArea();
			curr_sector->Acquire();
			curr_sector->Remove(id);
			curr_sector->Release();

			std::vector<PID> viewlist_curr = session->GetLocalSight();

			// ������ ����
			ReleaseClient(index, session);

			// ������ ���� �ִ� �þ� ����� �÷��̾�鿡�Լ� ĳ���� ����
			for (auto cit = viewlist_curr.cbegin(); viewlist_curr.cend() != cit; cit++)
			{
				const PID cid = *cit;
				if (cid == id) continue;

				auto other = AcquireClientByID(cid);
				if (other)
				{
					// �þ� ��Ͽ��� �ش� Ŭ���̾�Ʈ ����
					other->RemoveSight(id);
					if (SOCKET_ERROR == SendDisppearEntity(other, id))
					{
						if (WSA_IO_PENDING != WSAGetLastError())
						{
							ErrorDisplay("Disconnect()");
							std::cout << "Ŭ���̾�Ʈ " << id << "���� ����!\n";
						}
					}

					ReleaseClient(other->Index, other);
				}
			}

			// �������� �ش� Ŭ���̾�Ʈ�� ����
			DeregisterPlayer(id);

			// Broadcast: Ŭ���̾�Ʈ���� ���� ���Ḧ ����
			//for (auto& player : myClients)
			{
				//auto other = GetClient(player.second);
				//SendSignOut(other, id);
			}
			// ���� Ŭ���̾�Ʈ�� �ִ� ���� û��
			session->Cleanup();
		}
		else if (session->IsConnected())
		{
			// �������� �ش� Ŭ���̾�Ʈ�� ����
			DeregisterPlayer(id);
			// ���� Ŭ���̾�Ʈ�� �ִ� ���� û��
			session->Cleanup();
			// ������ ����
			ReleaseClient(index, session);
		}
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
	//myClients.unsafe_erase(myClients.find(rid));
	myClients.erase(rid);
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

void IOCPFramework::InitializeWorldFor(const UINT index, SessionPtr& who)
{
	// �þ� ����� ���� (��� ����),
	UpdateSightOf(index);
}

template<typename MY_PACKET, typename ...Ty>
	requires std::is_base_of_v<Packet, MY_PACKET>
std::pair<LPWSABUF, Asynchron*> IOCPFramework::CreateTicket(Ty&&... args) const
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

int IOCPFramework::SendSignUp(SessionPtr& target, const PID id) const
{
	std::cout << target->GetID() << " �� SendSignUp(" << id << ")\n";

	const auto ticket = CreateTicket<SCPacketSignUp>(id, 0, GetClientsNumber());
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendPlayerCreate(SessionPtr& target, const PID who, char* nickname) const
{
	std::cout << target->GetID() << " �� SendPlayerCreate(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketCreatePlayer>(who, nickname);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendSignOut(SessionPtr& target, const PID who) const
{
	std::cout << target->GetID() << " �� SendSignOut(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketSignOut>(who, GetClientsNumber());
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendAppearEntity(SessionPtr& target, PID cid, ENTITY_TYPES type, float cx, float cy) const
{
	std::cout << target->GetID() << " �� SendAppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketAppearCharacter>(cid, type, cx, cy);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendDisppearEntity(SessionPtr& target, PID cid) const
{
	std::cout << target->GetID() << " �� SendDisppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketDisppearCharacter>(cid);
	return std::forward<SessionPtr>(target)->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendMoveEntity(SessionPtr& target, PID cid, float nx, float ny) const
{
	std::cout << target->GetID() << " �� SendMoveEntity(" << cid << ")\n";

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
	auto it = std::find_if(acceptBeginPlace, clientsPool.cend()
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
