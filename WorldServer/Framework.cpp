#include "pch.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Session.h"
#include "PlayingSession.hpp"
#include "SightManager.hpp"
#include "SightSector.hpp"
#include "Timeline.hpp"

int SightDistance(const float pos1[2], const float pos2[2])
{
	const auto x1 = int(pos1[0]);
	const auto y1 = int(pos1[1]);
	const auto x2 = int(pos2[0]);
	const auto y2 = int(pos2[1]);

	return int(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, serverKey(100)
	, clientsPool(), orderClientIDs(USERS_ID_BEGIN), numberClients(0), mutexClient()
	, threadWorkers(THREADS_COUNT)
	, mySightManager(*this, WORLD_W, WORLD_H, SIGHT_W, SIGHT_H)
	, myCharacterDatas(0, ENTITY_CATEGORY::NONE, ENTITY_TYPES::NONE, 0, 0)
	, timerQueue(), timerMutex()
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	BuildSession();
}

IOCPFramework::~IOCPFramework()
{
	CloseHandle(completionPort);
	closesocket(Listener);
	WSACleanup();
}

void IOCPFramework::BuildSession()
{
	// 앞쪽은 비플레이어 세션
	for (auto i = NPC_ID_BEGIN; i < NPC_ID_END; ++i)
	{
		auto& empty = clientsPool.at(i);
		empty = std::make_shared<Session>(i, -1, *this);
	}

	// 뒤쪽은 플레이어 세션 (USERS_MAX)
	for (auto j = USERS_ID_BEGIN; j < USERS_ID_END; ++j)
	{
		auto& empty = clientsPool.at(j);

		auto player = std::make_shared<PlayingSession>(j, -1, *this);
		empty = std::static_pointer_cast<Session>(player);
	}

	acceptBeginPlace = clientsPool.cbegin() + CLIENTS_ORDER_BEGIN;
}

void IOCPFramework::BuildNPC()
{
	for (auto i = NPC_ID_BEGIN; i < NPC_ID_END; ++i)
	{
		auto npc = AcquireClient(i);
		auto state = npc->AcquireStatus();

		auto npc_avatar = make_shared<GameObject>(PID(i), 4.0f, 4.0f);
		npc_avatar->myPosition[0] = float(rand() % int(WORLD_W));
		npc_avatar->myPosition[1] = float(rand() % int(WORLD_H));

		npc->SetAvatar(npc_avatar);
		npc->myNickname = "M-" + std::to_string(i);

		npc->ReleaseStatus(SESSION_STATES::ACCEPTED);
		CreateSight(npc);

		ReleaseClient(i, npc);
	}
}

void IOCPFramework::Awake()
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
		ErrorDisplay("Awake → WSASocket()");
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

	if (SOCKET_ERROR == listen(Listener, USERS_MAX))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "서버 시작\n";

	acceptNewbie.store(CreateSocket(), std::memory_order_release);

	BuildNPC();

	Listen();
}

void IOCPFramework::Update()
{
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::IOCPWorker);
	threadWorkers.emplace_back(::AIWorker);
	threadWorkers.emplace_back(::TimerWorker);

	while (true) {}
}

void IOCPFramework::Release()
{
	std::cout << "서버 종료\n";
}

void IOCPFramework::Communicate()
{
	DWORD portBytes = 0;
	ULONG_PTR portKey = 0;
	WSAOVERLAPPED* portOverlap = nullptr;
	const auto port = completionPort;

	auto result = GetQueuedCompletionStatus(port, &portBytes, &portKey, &portOverlap, INFINITE);

	if (TRUE == result)
	{
		std::cout << "GQCS: " << portKey << ", Bytes: " << portBytes << "\n";

		if (serverKey == portKey) // AcceptEx
		{
			ProceedAccept();
		}

		else
		{
			ProceedOperations(portOverlap, portKey, portBytes);
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

void IOCPFramework::ProceedAccept()
{
	auto number = GetClientsNumber();
	if (USERS_MAX <= number)
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
				session->SetID(key);
				session->SetSocket(newbie);
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

void IOCPFramework::ProceedOperations(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	// ID
	const auto my_id = static_cast<PID>(key);

	// NPC 또는 플레이어
	auto client = GetClientByID(my_id);

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
			ErrorDisplay("ProceedOperations(bytes=0)");
			return;
		}

		auto op = exoverlap->Operation;

		switch (op)
		{
			case OVERLAP_OPS::NONE:
			{}
			break;

			case OVERLAP_OPS::RECV: // 플레이어
			{
				client->ProceedReceived(exoverlap, bytes);
			}
			break;

			case OVERLAP_OPS::SEND: // 플레이어
			{
				client->ProceedSent(exoverlap, bytes);
			}
			break;

			case OVERLAP_OPS::ENTITY_MOVE: // 플레이어 또는 NPC
			{
				bool moved = false;
				auto dir = static_cast<MOVE_TYPES>(bytes);
				auto avatar = client->AcquireAvatar();

				switch (dir)
				{
					case MOVE_TYPES::LEFT:
					{
						moved = avatar->TryMoveLT(CELL_W);
					}
					break;

					case MOVE_TYPES::RIGHT:
					{
						moved = avatar->TryMoveRT(CELL_W);
					}
					break;

					case MOVE_TYPES::UP:
					{
						moved = avatar->TryMoveUP(CELL_H);
					}
					break;

					case MOVE_TYPES::DOWN:
					{
						moved = avatar->TryMoveDW(CELL_H);
					}
					break;

					default:
					break;
				}

				if (moved)
				{
					std::cout << "캐릭터 " << my_id
						<< " - 위치: ("
						<< avatar->myPosition[0] << ", " << avatar->myPosition[1]
						<< ")\n";

					// 시야 정보 전송
					UpdateSightOf(client->Index);
				}
				else
				{
					std::cout << "캐릭터 " << my_id << " - 움직이지 않음.\n";
				}

				client->ReleaseAvatar(avatar);
			}
			break;

			case OVERLAP_OPS::ENTITY_ATTACK: // NPC 평타
			{/*}

	bool attacked = false;
	auto avatar = AcquireAvatar();
	const auto place = avatar->GetPosition();

	const auto& my_zone = myFramework.mySightManager.AtByPosition(place[0], place[1]);
	my_zone->Acquire();

	const auto& sight_list = GetSight();

	if (1 < sight_list.size()) // 자기 자신도 포함
	{
		for (const auto& pid : sight_list)
		{
			if (pid == ID)
			{
				continue;
			}

			auto other = myFramework.AcquireClientByID(pid);
			auto other_id = other->AcquireID();
			auto other_avatar = other->AcquireAvatar();



			other->ReleaseAvatar(other_avatar);
			other->ReleaseID(other_id);
			myFramework.ReleaseClient(other->Index, other);
		}
	}

	switch (dir)
	{
		case MOVE_TYPES::LEFT:
		{
			attacked = avatar->TryMoveLT(CELL_W);
		}
		break;

		case MOVE_TYPES::RIGHT:
		{
			attacked = avatar->TryMoveRT(CELL_W);
		}
		break;

		case MOVE_TYPES::UP:
		{
			attacked = avatar->TryMoveUP(CELL_H);
		}
		break;

		case MOVE_TYPES::DOWN:
		{
			attacked = avatar->TryMoveDW(CELL_H);
		}
		break;

		default:
		break;
	}

	ReleaseAvatar(avatar);
	my_zone->Release();
*/

			}
			break;

			case OVERLAP_OPS::PLAYER_ATTACK: // 플레이어 평타
			{

			}
			break;

			case OVERLAP_OPS::ENTITY_DEAD:
			{

			}
			break;
		}
	}
}


void IOCPFramework::CreateSight(shared_ptr<Session> who)
{
	const auto my_id = who->AcquireID();
	const auto my_character = who->AcquireAvatar();
	const auto& my_pos = my_character->GetPosition();

	const auto curr_coords = mySightManager.PickCoords(my_pos[0], my_pos[1]);
	auto& curr_sector = mySightManager.At(curr_coords);

	curr_sector->Acquire();
	curr_sector->Add(my_id);
	who->SetSightArea(curr_sector);
	curr_sector->Release();

	constexpr auto sight_magnitude = SIGHT_RAD_W * SIGHT_RAD_H;
	for (UINT other_index = 0; other_index < ENTITIES_MAX_NUMBER; other_index++)
	{
		auto other = AcquireClient(other_index);

		if (other && other->IsAccepted())
		{
			const auto other_id = other->AcquireID();
			const auto other_avatar = other->AcquireAvatar();
			const auto other_pos = other_avatar->GetPosition();

			const bool check_out = sight_magnitude < SightDistance(my_pos, other_pos);

			if (check_out)
			{
				other->ReleaseID(other_id);
				continue;
			}

			if (IsPlayer(other_index))
			{
				if (my_id != other_id)
				{
					// 개체로부터 속성 따오기
					SCPacketAppearEntity other_data{ other_id, other_avatar->myCategory, other_avatar->myType, other_pos[0], other_pos[1] };

					who->AddSight(other_id);
					SendAppearEntity(who, other_id, other_data);
				}
				const auto& my_category = my_character->myCategory;
				const auto& my_type = my_character->myType;

				// 전송할 나의 속성 만들기
				myCharacterDatas = SCPacketAppearEntity{ my_id, my_category, my_type, my_pos[0], my_pos[1] };
				myCharacterDatas.x = my_pos[0];
				myCharacterDatas.y = my_pos[1];
				myCharacterDatas.level = my_character->myLevel;
				myCharacterDatas.hp = my_character->myHP;
				myCharacterDatas.maxhp = my_character->myMaxHP;
				myCharacterDatas.mp = my_character->myMP;
				myCharacterDatas.maxmp = my_character->myMaxMP;
				myCharacterDatas.amour = my_character->myArmour;

				other->AddSight(my_id);
				SendAppearEntity(other, my_id, myCharacterDatas);
			}
			else
			{
				// 개체로부터 속성 따오기
				SCPacketAppearEntity other_data{ other_id, other_avatar->myCategory, other_avatar->myType, other_pos[0], other_pos[1] };

				who->AddSight(other_id);
				SendAppearEntity(who, other_id, other_data);

				other->AddSight(my_id);
			}

			other->ReleaseAvatar(other_avatar);
			other->ReleaseID(other_id);
			ReleaseClient(other_index, other);
		}

		ReleaseClient(other_index, other);
	}

	who->ReleaseAvatar(my_character);
	who->ReleaseID(my_id);
}

void IOCPFramework::RemoveSight(const PID id)
{
	auto session = GetClientByID(id);

	// 시야 구역에서 해당 클라이언트를 삭제
	auto& curr_sector = session->GetSightArea();
	curr_sector->Acquire();
	curr_sector->Remove(id);
	curr_sector->Release();

	// 세션이 갖고 있는 시야 목록의 플레이어들에게서 캐릭터 삭제
	auto viewlist_curr = session->GetLocalSight();

	for (auto& other_id : viewlist_curr)
	//for (auto other_index = USERS_ID_BEGIN; other_index < USERS_ID_END; other_index++)
	{
		//auto other = AcquireClient(other_index);

		auto other = AcquireClientByID(other_id);

		if (other && other->IsAccepted())
		{
			other->RemoveSight(id);

			if (IsPlayer(other_id)
				&& SOCKET_ERROR == SendDisppearEntity(other, id))
			{
				if (WSA_IO_PENDING != WSAGetLastError())
				{
					ErrorDisplay("Disconnect()");
					std::cout << "클라이언트 " << id << "에서 오류!\n";
				}
			}
		}

		//ReleaseClient(other_index, other);
		ReleaseClient(other->Index, other);
	}
}

void IOCPFramework::UpdateSightOf(const UINT index)
{
	auto session = AcquireClient(index);

	// NPC, 특수 개체, 플레이어의 고유 식별자
	const PID my_id = session->AcquireID();

	// 예전 시야
	auto viewlist_prev = session->GetLocalSight();

	const auto& my_character = session->AcquireAvatar();
	const auto& my_pos = my_character->GetPosition();

	// 0. 자기 캐릭터가 속한 구역을 갱신한다.
	const auto curr_pos = float_pair(my_pos[0], my_pos[1]);
	const auto curr_coords = mySightManager.PickCoords(curr_pos);
	auto& curr_sector = mySightManager.At(curr_coords);

	// 적법한 구역의 소유권 획득
	curr_sector->Acquire();

	// 다른 메서드라면 소유권 획득이 안된다
	auto& prev_sector = session->GetSightArea();
	if (nullptr == prev_sector || prev_sector->TryAcquire())
	{
		if (curr_sector != prev_sector)
		{
			// 이전 시야 구역은 해제
			if (prev_sector)
			{
				prev_sector->Remove(my_id);
			}
			// 시야 구역에 등록
			curr_sector->Add(my_id);
			session->SetSightArea(curr_sector);
		}

		// 내가 소유한 구역만 소유권 내려놓기
		prev_sector->Release();
	}

	// 1. 자기가 속한 구역의 목록을 얻는다.
	auto viewlist_curr = curr_sector->GetSightList();

	// 2. 시야 사각형에 닿는 구역들을 찾는다.

	constexpr auto sgh_w = SIGHT_CELLS_RAD_H;
	constexpr auto sgh_h = SIGHT_CELLS_RAD_V;

	// 3. 각 구역의 시야 목록을 더한다.

	const auto lu_coords = curr_coords + int_pair{ -sgh_w, -sgh_h };
	if (0 < lu_coords.first && 0 < lu_coords.second)
	{
		const auto& list = mySightManager.At(lu_coords)->GetSightList();

		viewlist_curr.insert(list.begin(), list.end());
		//std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
		//	viewlist_curr.insert(ot_id);
		//});
	}

	const auto ru_coords = curr_coords + int_pair{ +sgh_w, -sgh_h };
	if (ru_coords.first < WORLD_CELLS_CNT_H && 0 < ru_coords.second)
	{
		const auto& list = mySightManager.At(ru_coords)->GetSightList();

		viewlist_curr.insert(list.begin(), list.end());
		///std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
		//	viewlist_curr.insert(ot_id);
		//});
	}

	const auto ld_coords = curr_coords + int_pair{ -sgh_w, +sgh_h };
	if (0 < ld_coords.first && ld_coords.second < WORLD_CELLS_CNT_V)
	{
		const auto& list = mySightManager.At(ld_coords)->GetSightList();

		viewlist_curr.insert(list.begin(), list.end());
		//std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
		//	viewlist_curr.insert(ot_id);
		//});
	}

	const auto rd_coords = curr_coords + int_pair{ +sgh_w, +sgh_h };
	if (rd_coords.first < WORLD_CELLS_CNT_H && rd_coords.second < WORLD_CELLS_CNT_V)
	{
		const auto& list = mySightManager.At(rd_coords)->GetSightList();

		viewlist_curr.insert(list.begin(), list.end());
		//std::for_each(list.begin(), list.end(), [&](const PID& ot_id) {
		//	viewlist_curr.insert(ot_id);
		//});
	}
	curr_sector->Release();

	const auto& my_category = my_character->myCategory;
	const auto& my_type = my_character->myType;

	// 전송할 나의 속성 만들기
	myCharacterDatas = SCPacketAppearEntity{ my_id, my_category, my_type, my_pos[0], my_pos[1] };
	myCharacterDatas.x = my_pos[0];
	myCharacterDatas.y = my_pos[1];
	myCharacterDatas.level = my_character->myLevel;
	myCharacterDatas.hp = my_character->myHP;
	myCharacterDatas.maxhp = my_character->myMaxHP;
	myCharacterDatas.mp = my_character->myMP;
	myCharacterDatas.maxmp = my_character->myMaxMP;
	myCharacterDatas.amour = my_character->myArmour;

	// 4. 새로운 시야 목록 할당
	session->AssignSight(viewlist_curr);

	// 5. 시야 목록의 차이점을 전송
	// * 현재 없는 개체는 Disappear
	// * 현재 있는 개체는, 과거에도 있으면 Move, 없으면 Appear
	PID other_id;

	constexpr auto sight_magnitude = SIGHT_RAD_W * SIGHT_RAD_H;

	for (auto cit = viewlist_curr.cbegin(); viewlist_curr.cend() != cit;)
	{
		other_id = *cit;

		auto other = AcquireClientByID(other_id);
		if (!other)
		{
			session->RemoveSight(other_id);
			cit++;
			continue;
		}

		const auto& other_avatar = other->AcquireAvatar();
		const auto& ot_pos = other_avatar->GetPosition();
		const bool check_out = sight_magnitude < SightDistance(my_pos, ot_pos);
		const bool ot_is_player = IsPlayer(other_id);

		const auto pit = viewlist_prev.find(other_id);

		if (check_out)
		{
			session->RemoveSight(other_id);

			SendDisppearEntity(session, other_id);

			if (other_id != my_id && ot_is_player)
			{
				other->RemoveSight(my_id);

				SendDisppearEntity(other, my_id);
			}

			cit++;
		}
		else if (viewlist_prev.cend() == pit)
		{
			// Appear: 새로운 개체 등록
			if (other_id != my_id)
			{
				session->AddSight(other_id);

				// 개체로부터 속성 따오기
				SCPacketAppearEntity other_data{ other_id, other_avatar->myCategory, other_avatar->myType, ot_pos[0], ot_pos[1] };

				SendAppearEntity(session, other_id, other_data);

				// 상대도 개체 등록
				if (ot_is_player)
				{
					other->AddSight(my_id);

					SendAppearEntity(other, my_id, myCharacterDatas);
				}
			}
			else
			{
				SendAppearEntity(other, my_id, myCharacterDatas);
			}

			cit++;
		}
		else
		{
			// Move
			SendMoveEntity(session, other_id, ot_pos[0], ot_pos[1], other_avatar->myDirection);

			if (other_id != my_id && ot_is_player)
			{
				SendMoveEntity(other, my_id, my_pos[0], my_pos[1], my_character->myDirection);
			}

			viewlist_prev.erase(pit);
			cit++;
		}

		other->ReleaseAvatar(other_avatar);
		ReleaseClient(other->Index, other);
	}

	// Disappear
	for (auto pit = viewlist_prev.cbegin(); viewlist_prev.cend() != pit;
		++pit)
	{
		other_id = *pit;

		auto other = GetClientByID(other_id);
		const bool ot_is_player = IsPlayer(other_id);

		session->RemoveSight(other_id);
		SendDisppearEntity(session, other_id);

		if (other_id != my_id && ot_is_player)
		{
			other->RemoveSight(my_id);
			SendDisppearEntity(other, my_id);
		}
	}

	session->ReleaseID(my_id);
	session->ReleaseAvatar(my_character);

	ReleaseClient(index, session);
}

SessionPtr IOCPFramework::CreateNPC(const UINT index, ENTITY_CATEGORY type, int info_index)
{
	return SessionPtr();
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
			RemoveSight(id);

			// 서버에서 해당 클라이언트를 삭제
			DeregisterPlayer(id);

			// Broadcast: 클라이언트에게 접속 종료를 통지
			//for (auto& player : myClients)
			{
				//auto other = GetClient(player.second);
				//SendSignOut(other, id);
			}

			// 원래 클라이언트가 있던 세션 청소
			session->Cleanup();
			// 소유권 해제
			ReleaseClient(index, session);
		}
		else if (session->IsConnected())
		{
			// 서버에서 해당 클라이언트를 삭제
			DeregisterPlayer(id);

			// 원래 클라이언트가 있던 세션 청소
			session->Cleanup();
			// 소유권 해제
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

void IOCPFramework::InitializeWorldFor(const UINT index, SessionPtr& who)
{
	CreateSight(who);
	//UpdateSightOf(index);
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
	std::cout << target->GetID() << " → SendSignUp(" << id << ")\n";

	const auto ticket = CreateTicket<SCPacketSignUp>(id, 0, GetClientsNumber());
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendSignInFailed(SessionPtr& target, LOGIN_ERROR_TYPES type) const
{
	std::cout << target->GetID() << " → SendSignInFailed()\n";

	const auto ticket = CreateTicket<SCPacketSignInFailed>(target->GetID(), type, GetClientsNumber());
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendPlayerCreate(SessionPtr& target, const PID who, char* nickname) const
{
	std::cout << target->GetID() << " → SendPlayerCreate(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketCreatePlayer>(who, nickname);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendSignOut(SessionPtr& target, const PID who) const
{
	std::cout << target->GetID() << " → SendSignOut(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketSignOut>(who);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendAppearEntity(SessionPtr& target, PID cid, SCPacketAppearEntity data) const
{
	std::cout << target->GetID() << " → SendAppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketAppearEntity>(data);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendDisppearEntity(SessionPtr& target, PID cid) const
{
	std::cout << target->GetID() << " → SendDisppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketDisppearCharacter>(cid);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendMoveEntity(SessionPtr& target, PID cid, float nx, float ny, MOVE_TYPES dir) const
{
	std::cout << target->GetID() << " → SendMoveEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketMoveCharacter>(cid, nx, ny, dir);
	return target->Send(ticket.first, 1, ticket.second);
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
	const auto it = myClients.find(id);
	if (myClients.cend() != it)
	{
		return AcquireClient(it->second);
	}
	else
	{
		return nullptr;
	}
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

bool IOCPFramework::IsClientsBound(const UINT index) const
{
	return (0 <= index && index < clientsPool.size());
}

SOCKET IOCPFramework::CreateSocket() const volatile
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED);
}
