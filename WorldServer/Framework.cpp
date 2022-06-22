#include "pch.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Session.h"
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
	: myListener(NULL), myAddress(), szAddress()
	, completionPort(NULL), serverKey(ENTITIES_MAX_NUMBER + 1)
	, acceptOverlap(OVERLAP_OPS::LISTEN), acceptBytes(0), acceptCBuffer()
	, clientsPool(), orderClientIDs(USERS_ID_BEGIN), numberClients(0)
	, mySightManager(*this, WORLD_W, WORLD_H, SIGHT_W, SIGHT_H)
	, threadWorkers(THREADS_COUNT)
	, timerQueue(), timerMutex()
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
}

IOCPFramework::~IOCPFramework()
{
	CloseHandle(completionPort);
	closesocket(myListener);
	WSACleanup();
}

void IOCPFramework::Awake()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	myListener = CreateSocket();
	if (INVALID_SOCKET == myListener)
	{
		ErrorDisplay("Awake → WSASocket()");
		return;
	}

	szAddress = sizeof(myAddress);
	ZeroMemory(&myAddress, szAddress);
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddress.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(myListener, (SOCKADDR*)(&myAddress), szAddress))
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

	auto apply = CreateIoCompletionPort(HANDLE(myListener), completionPort, serverKey, 0);
	if (NULL == apply)
	{
		ErrorDisplay("CreateIoCompletionPort(Listener)");
		return;
	}
}

void IOCPFramework::Start()
{
	BOOL option = TRUE;
	if (SOCKET_ERROR == setsockopt(myListener, SOL_SOCKET, SO_REUSEADDR
		, reinterpret_cast<char*>(&option), sizeof(option)))
	{
		ErrorDisplay("Start → setsockopt(Listener)");
		return;
	}

	if (SOCKET_ERROR == listen(myListener, USERS_MAX))
	{
		ErrorDisplay("listen()");
		return;
	}

	acceptNewbie.store(CreateSocket(), std::memory_order_release);

	std::cout << "세션 구축 중...";
	BuildSessions();
	std::cout << " 완료!\n";

	std::cout << "NPC 구축 중...";
	BuildNPCs();
	std::cout << " 완료!\n";

	std::cout << "서버 시작\n";
	Listen();

	BuildThreads();
}

void IOCPFramework::Update()
{
	DWORD portBytes = 0;
	ULONG_PTR portKey = 0;
	WSAOVERLAPPED* portOverlap = nullptr;
	const auto port = completionPort;

	RECT tempBoundingBox{};

	auto result = GetQueuedCompletionStatus(port, &portBytes, &portKey, &portOverlap, INFINITE);

	if (TRUE == result)
	{
		auto exoverlap = static_cast<Asynchron*>(portOverlap);
		const auto my_op = exoverlap->Operation;
		const auto my_id = static_cast<PID>(portKey);

		switch (my_op)
		{
			case OVERLAP_OPS::NONE:
			{}
			break;

			case OVERLAP_OPS::LISTEN:
			{
				ProceedSignUp();
			}
			break;

			case OVERLAP_OPS::ACCEPT:
			{
				delete exoverlap;
			}
			break;

			case OVERLAP_OPS::RECV: // 플레이어
			{
				std::cout << "GQCS (" << static_cast<int>(my_op) << "): "
					<< "키 값: " << portKey
					<< ", 받은 바이트 수: " << portBytes << "\n";

				auto session = GetClientByID(my_id);
				session->ProceedReceived(exoverlap, portBytes);
			}
			break;

			case OVERLAP_OPS::SEND: // 플레이어
			{
				auto session = GetClientByID(my_id);
				session->ProceedSent(exoverlap, portBytes);
			}
			break;

			case OVERLAP_OPS::ENTITY_MOVE: // 플레이어 또는 NPC
			{
				auto session = GetClientByID(my_id);

				bool moved = false;
				auto dir = static_cast<MOVE_TYPES>(portBytes);

				switch (dir)
				{
					case MOVE_TYPES::LEFT:
					{
						moved = session->TryMoveLT(CELL_W);
					}
					break;

					case MOVE_TYPES::RIGHT:
					{
						moved = session->TryMoveRT(CELL_W);
					}
					break;

					case MOVE_TYPES::UP:
					{
						moved = session->TryMoveUP(CELL_H);
					}
					break;

					case MOVE_TYPES::DOWN:
					{
						moved = session->TryMoveDW(CELL_H);
					}
					break;

					default:
					break;
				}

				if (moved)
				{
					std::cout << "캐릭터 " << my_id
						<< " - 위치: ("
						<< session->myPosition[0] << ", " << session->myPosition[1]
						<< ")\n";

					// 시야 정보 전송
					UpdateSight(session.get());
				}
				else
				{
					std::cout << "캐릭터 " << my_id << " - 움직이지 않음.\n";
				}
			}
			break;

			case OVERLAP_OPS::ENTITY_ATTACK: // NPC
			{
				auto session = GetClientByID(my_id);
				const auto dir = static_cast<MOVE_TYPES>(portBytes);

				if (session)
				{
					bool attacked = false;
					const auto place = session->GetPosition();

					const auto& my_zone = mySightManager.AtByPosition(place[0], place[1]);
					my_zone->Acquire();

					const auto& sight_list = session->GetSight();
					my_zone->Release();

					if (1 < sight_list.size()) // 자기 자신도 포함
					{
						switch (dir)
						{
							case MOVE_TYPES::LEFT:
							{
								tempBoundingBox;
							}
							break;

							case MOVE_TYPES::RIGHT:
							{

							}
							break;

							case MOVE_TYPES::UP:
							{

							}
							break;

							case MOVE_TYPES::DOWN:
							{

							}
							break;

							default:
							break;
						}

						const auto& packet_handle = exoverlap->sendBuffer->buf;

						for (const auto& pid : sight_list)
						{
							if (pid == my_id)
							{
								continue;
							}

							auto other = AcquireClientByID(pid);
							auto other_id = other->AcquireID();



							other->ReleaseID(other_id);
							ReleaseClient(other->Index, other);
						}
					}
				}

				delete exoverlap;
			}
			break;

			case OVERLAP_OPS::PLAYER_ATTACK: // 플레이어 평타
			{
				auto session = GetClientByID(my_id);
				const auto dir = static_cast<MOVE_TYPES>(portBytes);

				if (session)
				{
					bool attacked = false;
					const auto place = session->GetPosition();

					const auto& my_zone = mySightManager.AtByPosition(place[0], place[1]);
					my_zone->Acquire();

					const auto& sight_list = session->GetSight();
					my_zone->Release();

					if (1 < sight_list.size()) // 자기 자신도 포함
					{
						switch (dir)
						{
							case MOVE_TYPES::LEFT:
							{
								tempBoundingBox;
							}
							break;

							case MOVE_TYPES::RIGHT:
							{

							}
							break;

							case MOVE_TYPES::UP:
							{

							}
							break;

							case MOVE_TYPES::DOWN:
							{

							}
							break;

							default:
							break;
						}

						for (const auto& pid : sight_list)
						{
							if (pid == my_id)
							{
								continue;
							}

							auto other = AcquireClientByID(pid);
							auto other_id = other->AcquireID();

							if (session->CheckCollision(other.get()))
							{
								session->RemoveViewOf(other_id);
							}

							other->ReleaseID(other_id);
							ReleaseClient(other->Index, other);
						}
					}
				}

				delete exoverlap;
			}
			break;

			case OVERLAP_OPS::ENTITY_DEAD:
			{

			}
			break;

			default:
			{}
			break;
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

void IOCPFramework::Release()
{
	std::cout << "서버 종료\n";
}

void IOCPFramework::ProceedSignUp()
{
	auto newbie = AcquireNewbieSocket();

	auto number = GetClientsNumber();
	if (USERS_MAX <= number)
	{
		ProceedLoginFailed(newbie, LOGIN_ERROR_TYPES::USERS_LIMITED);
	}
	else
	{
		auto session = FindPlaceForNewbie();

		if (session)
		{
			auto newbie_id = AcquireNewbieID();
			const auto newbie_place = session->Index;

			auto io = CreateIoCompletionPort(HANDLE(newbie), completionPort, newbie_id, 0);
			if (NULL == io)
			{
				ErrorDisplay("ProceedSignUp → CreateIoCompletionPort()");
				std::cout << "클라이언트 " << newbie << "가 접속에 실패했습니다.\n";
				closesocket(newbie);
			}
			else
			{
				session->SetID(newbie_id);
				session->SetSocket(newbie);
				session->SetStatus(SESSION_STATES::CONNECTED);

				if (SOCKET_ERROR == session->RecvStream())
				{
					if (WSA_IO_PENDING != WSAGetLastError())
					{
						ErrorDisplay("ProceedSignUp → RecvStream()");
						std::cout << "클라이언트 " << newbie_id << "에서 오류!\n";
						ProceedLoginFailed(newbie, LOGIN_ERROR_TYPES::NETWORK_ERROR);
						session->Cleanup();
					}
				}
				else
				{
					RegisterPlayer(newbie_id, newbie_place);

					newbie_id++;
				}
			}

			// 클라이언트 ID의 소유권 내려놓기
			ReleaseNewbieID(newbie_id);
			// 클라이언트의 소유권 내려놓기
			ReleaseClient(newbie_place, session);
		}
		else
		{
			ProceedLoginFailed(newbie, LOGIN_ERROR_TYPES::NETWORK_ERROR);
		}
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	// acceptNewbie 소켓의 소유권 내려놓기
	ReleaseNewbieSocket(CreateSocket());

	Listen();
}

void IOCPFramework::ProceedLoginFailed(SOCKET sock, LOGIN_ERROR_TYPES reason)
{
	std::cout << "새로운 클라이언트 " << sock << "가 접속에 실패했습니다.\n";

	switch (reason)
	{
		case LOGIN_ERROR_TYPES::USERS_LIMITED:
		{
			std::cout << "새 접속을 받을 수 없습니다!\n";
		}
		break;

		case LOGIN_ERROR_TYPES::NETWORK_ERROR:
		{
			std::cout << "네트워크 오류가 발생했습니다!\n";
		}
		break;
	}

	auto packet = new SCPacketSignInFailed(0, reason, GetClientsNumber());

	auto wbuffer = new WSABUF{};
	wbuffer->buf = reinterpret_cast<char*>(packet);
	wbuffer->len = packet->Size;

	auto overlap = new Asynchron{ OVERLAP_OPS::SEND };
	overlap->Type = packet->Type;
	overlap->SetSendBuffer(wbuffer);

	std::cout << sock << " → SendSignInFailed()\n";
	int result = WSASend(sock, wbuffer, 1, NULL, 0, overlap, NULL);
	if (SOCKET_ERROR == result)
	{
		int error_code = WSAGetLastError();
		if (WSA_IO_PENDING != error_code)
		{
			std::cout << "소켓 " << sock << "의 연결 실패 전송이 실패함!\n";
		}
	}
}

void IOCPFramework::ProceedSignIn(Session* handle, const CSPacketSignIn* packet)
{
	auto my_id = handle->AcquireID();
	auto status = handle->AcquireStatus();

	if (SESSION_STATES::CONNECTED == status)
	{
		std::cout << "클라이언트 " << my_id << "접속함\n";
		handle->myNickname = packet->Nickname;
		std::cout << my_id << "'s Nickname: " << handle->myNickname << ".\n";

		handle->myPosition[0] = 100.0f;
		handle->myPosition[1] = 100.0f;

		// 클라이언트 ID 부여
		std::cout << "SendSignUp(" << my_id << ")\n";
		handle->SetID(my_id);

		const auto ticket = CreateTicket<SCPacketSignUp>(my_id, 0, GetClientsNumber());
		int result = handle->Send(ticket.first, 1, ticket.second);

		// 시야 정보 전송
		InitializeSight(handle);

		status = SESSION_STATES::ACCEPTED;
	}

	handle->ReleaseStatus(status);
	handle->ReleaseID(my_id);
}

void IOCPFramework::RegisterSight(Session* who)
{
	const auto my_id = who->AcquireID();
	const auto& my_pos = who->GetPosition();

	const auto curr_coords = mySightManager.PickCoords(my_pos[0], my_pos[1]);
	auto& curr_sector = mySightManager.At(curr_coords);

	curr_sector->Acquire();
	curr_sector->Add(my_id);
	who->SetSightArea(curr_sector);
	curr_sector->Release();

	if (IsPlayer(my_id))
	{
		who->myInfobox.x = who->myPosition[0];
		who->myInfobox.y = who->myPosition[1];

		SendAppearEntity(who, my_id, who->myInfobox);
	}

	who->ReleaseID(my_id);
}

void IOCPFramework::InitializeSight(Session* who)
{
	RegisterSight(who);

	UpdateSight(who);
}

void IOCPFramework::UpdateSight(Session* who)
{
	// NPC, 특수 개체, 플레이어의 고유 식별자
	const PID my_id = who->AcquireID();

	// 예전 시야
	auto viewlist_prev = who->GetLocalSight();

	const auto& my_pos = who->GetPosition();

	// 0. 자기 캐릭터가 속한 구역을 갱신한다.
	const auto curr_pos = float_pair(my_pos[0], my_pos[1]);
	const auto curr_coords = mySightManager.PickCoords(curr_pos);
	auto& curr_sector = mySightManager.At(curr_coords);

	// 적법한 구역의 소유권 획득
	curr_sector->Acquire();

	// 다른 메서드라면 소유권 획득이 안된다
	auto& prev_sector = who->GetSightArea();
	if (!prev_sector || prev_sector->TryAcquire())
	{
		if (curr_sector != prev_sector)
		{
			// 시야 구역에 등록
			curr_sector->Add(my_id);

			// 이전 시야 구역은 해제
			if (prev_sector)
			{
				prev_sector->Remove(my_id);

				// 이전 구역은 소유권 내려놓기
				prev_sector->Release();
			}

			who->SetSightArea(curr_sector);
		}
	}

	// 1. 자기가 속한 구역의 목록을 얻는다.
	auto viewlist_curr = curr_sector->GetSightList();
	curr_sector->Release();

	// 2. 시야 사각형에 닿는 구역들을 찾는다.

	constexpr auto sgh_w = SIGHT_CELLS_RAD_H;
	constexpr auto sgh_h = SIGHT_CELLS_RAD_V;

	// 3. 각 구역의 시야 목록을 더한다.

	const auto lu_coords = curr_coords + int_pair{ -sgh_w, -sgh_h };
	if (0 < lu_coords.first && 0 < lu_coords.second)
	{
		auto& sector = mySightManager.At(lu_coords);
		sector->Acquire();

		const auto& list = sector->GetSightList();
		viewlist_curr.insert(list.begin(), list.end());
		sector->Release();
	}

	const auto ru_coords = curr_coords + int_pair{ +sgh_w, -sgh_h };
	if (ru_coords.first < WORLD_CELLS_CNT_H && 0 < ru_coords.second)
	{
		auto& sector = mySightManager.At(ru_coords);
		sector->Acquire();

		const auto& list = sector->GetSightList();
		viewlist_curr.insert(list.begin(), list.end());
		sector->Release();
	}

	const auto ld_coords = curr_coords + int_pair{ -sgh_w, +sgh_h };
	if (0 < ld_coords.first && ld_coords.second < WORLD_CELLS_CNT_V)
	{
		auto& sector = mySightManager.At(ld_coords);
		sector->Acquire();

		const auto& list = sector->GetSightList();
		viewlist_curr.insert(list.begin(), list.end());
		sector->Release();
	}

	const auto rd_coords = curr_coords + int_pair{ +sgh_w, +sgh_h };
	if (rd_coords.first < WORLD_CELLS_CNT_H && rd_coords.second < WORLD_CELLS_CNT_V)
	{
		auto& sector = mySightManager.At(rd_coords);
		sector->Acquire();

		const auto& list = sector->GetSightList();
		viewlist_curr.insert(list.begin(), list.end());
		sector->Release();
	}

	// 전송할 나의 속성 만들기
	auto& my_infobox = who->myInfobox;
	my_infobox.x = my_pos[0];
	my_infobox.y = my_pos[1];

	// 4. 새로운 시야 목록 할당
	who->AssignSight(viewlist_curr);

	constexpr auto sight_magnitude = SIGHT_RAD_W * SIGHT_RAD_H;

	// 5. 시야 목록의 차이점을 전송
	// * 현재 없는 개체는 Disappear
	// * 밖으로 나간 개체는 Disappear
	// * 현재 있는 개체는, 과거에도 있으면 Move, 없으면 Appear
	for (auto cit = viewlist_curr.cbegin(); viewlist_curr.cend() != cit;)
	{
		const PID other_id = *cit;

		// AcquireClientByID
		auto other = GetClientByID(other_id);
		if (!other)
		{
			std::cout << who << "의 시야에서 세션 " << other_id << "를 찾을 수 없었음.\n";

			who->RemoveViewOf(other_id);
			SendDisppearEntity(who, other_id);

			cit++;
			continue;
		}

		const auto& ot_pos = other->GetPosition();
		const bool check_out = sight_magnitude < SightDistance(my_pos, ot_pos);
		const bool ot_is_player = IsPlayer(other_id);

		// 이전 시야 목록에 other가 있었는지 확인
		const auto pit = viewlist_prev.find(other_id);

		if (viewlist_prev.cend() == pit) // * 현재 있는 개체는 예전에 없었으면 Appear
		{
			// Appear: 새로운 개체 등록
			if (other_id != my_id && !check_out)
			{
				who->AddSight(other_id);

				// 개체로부터 속성 따오기
				const auto& other_infobox = other->myInfobox;

				SendAppearEntity(who, other_id, other_infobox);

				// 상대도 개체 등록
				if (other_id != my_id && ot_is_player)
				{
					other->AddSight(my_id);

					SendAppearEntity(other.get(), my_id, my_infobox);
				}
			}
			else
			{
				// RegisterSight 했음
				//SendAppearEntity(other, my_id, myCharacterDatas);
			}

			cit++;
		}
		else if (check_out) // * 밖으로 나간 개체는 Disappear
		{
			who->RemoveViewOf(other_id);

			SendDisppearEntity(who, other_id);

			if (other_id != my_id && ot_is_player)
			{
				other->RemoveViewOf(my_id);

				SendDisppearEntity(other.get(), my_id);
			}

			// 이미 처리했으므로 제거
			viewlist_prev.erase(pit);
			cit++;
		}
		else  // * 현재 있는 개체는 과거에도 있었으면 Move
		{
			// Move
			SendMoveEntity(who, other_id, ot_pos[0], ot_pos[1], other->myDirection);

			if (ot_is_player)
			{
				SendMoveEntity(other.get(), my_id, my_pos[0], my_pos[1], who->myDirection);
			}

			// 예전 목록에서 현재 있는 개체들을 제거
			viewlist_prev.erase(pit);
			cit++;
		}

		//ReleaseClient(other->Index, other);
	}
	who->ReleaseID(my_id);

	 // * 현재 없는 개체는 Disappear
	for (auto pit = viewlist_prev.cbegin(); viewlist_prev.cend() != pit;
		++pit)
	{
		// Disappear
		const PID other_id = *pit;

		if (other_id != my_id)
		{
			// AcquireClientByID
			auto other = GetClientByID(other_id);
			const bool ot_is_player = IsPlayer(other_id);

			who->RemoveViewOf(other_id);

			SendDisppearEntity(who, other_id);

			if (ot_is_player)
			{
				other->RemoveViewOf(my_id);

				SendDisppearEntity(other.get(), my_id);
			}

			//ReleaseClient(other->Index, other);
		}
	}
}

void IOCPFramework::CleanupSight(Session* who)
{
	const auto my_id = who->GetID();

	// 시야 구역에서 해당 클라이언트를 삭제
	auto& curr_sector = who->GetSightArea();
	curr_sector->Acquire();
	curr_sector->Remove(my_id);
	curr_sector->Release();

	// 세션이 갖고 있는 시야 목록의 플레이어들에게서 캐릭터 삭제
	auto& viewlist_curr = who->GetSight();

	for (auto& other_id : viewlist_curr)
	{
		if (my_id == other_id) continue;

		auto other = AcquireClientByID(other_id);

		if (other && other->IsAccepted())
		{
			other->RemoveViewOf(my_id);

			if (IsPlayer(other_id))
			{
				SendDisppearEntity(other.get(), my_id);
			}
		}

		ReleaseClient(other->Index, other);
	}
}

void IOCPFramework::BuildSessions()
{
	PID npc_id = 0;
	for (UINT i = 0; i < ENTITIES_MAX_NUMBER; i++)
	{
		auto& empty_place = clientsPool.at(i);

		if (NPC_ID_BEGIN <= i && i < NPC_ID_END)
		{
			auto session = std::make_shared<Session>(i, npc_id++, *this);
			session->SetBoundingBox(-16, -16, 16, 16);

			empty_place = session;	
		}
		else
		{
			auto session = std::make_shared<Session>(i, PID(-1), *this);
			session->SetBoundingBox(-16, -16, 16, 16);

			empty_place = session;
		}
	}

	acceptBeginPlace = clientsPool.cbegin() + CLIENTS_ORDER_BEGIN;
}

void IOCPFramework::BuildNPCs()
{
	for (auto i = NPC_ID_BEGIN; i < NPC_ID_END; ++i)
	{
		auto npc = CreateNPC(i, ENTITY_CATEGORY::MOB);

		const auto cx = float(rand()) / float(RAND_MAX) * WORLD_W;
		const auto cy = float(rand()) / float(RAND_MAX) * WORLD_H;

		npc->SetPosition(cx, cy);
		RegisterSight(npc.get());

		npc->myNickname = "M-" + std::to_string(i);
	}
}

void IOCPFramework::BuildThreads()
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

void IOCPFramework::Listen()
{
	auto newbie = acceptNewbie.load(std::memory_order_relaxed);
	auto result = AcceptEx(myListener, newbie, acceptCBuffer
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

SessionPtr IOCPFramework::CreateNPC(const UINT index, ENTITY_CATEGORY type)
{
	const auto npc_id = PID(index);

	auto npc = AcquireClient(index);
	auto state = npc->AcquireStatus();

	npc->SetID(npc_id);
	npc->myCategory = type;

	//auto my_lua = luaL_newstate();
	//npc->myLuaMachine = my_lua;
	//luaL_openlibs(my_lua);

	// 등록
	myClients.insert({ npc_id, index });

	npc->ReleaseStatus(SESSION_STATES::ACCEPTED);
	ReleaseClient(index, npc);

	return npc;
}

void IOCPFramework::Disconnect(const PID id)
{
	if (auto session = AcquireClientByID(id); session)
	{
		const auto index = session->Index;

		if (session->IsAccepted())
		{
			CleanupSight(session.get());

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
		}
		else if (session->IsConnected())
		{
			// 서버에서 해당 클라이언트를 삭제
			DeregisterPlayer(id);

			// 원래 클라이언트가 있던 세션 청소
			session->Cleanup();
		}

		// 소유권 해제
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

	myClients.erase(rid);

	ReleaseClientsNumber(value - 1);
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

int IOCPFramework::SendPlayerCreate(Session* target, const PID who, char* nickname) const
{
	std::cout << target->GetID() << " → SendPlayerCreate(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketCreatePlayer>(who, nickname);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendSignOut(Session* target, const PID who) const
{
	std::cout << target->GetID() << " → SendSignOut(" << who << ")\n";

	const auto ticket = CreateTicket<SCPacketSignOut>(who);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendAppearEntity(Session* target, PID cid, SCPacketAppearEntity data) const
{
	std::cout << target->GetID() << " → SendAppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketAppearEntity>(data);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendDisppearEntity(Session* target, PID cid) const
{
	std::cout << target->GetID() << " → SendDisppearEntity(" << cid << ")\n";

	const auto ticket = CreateTicket<SCPacketDisppearCharacter>(cid);
	return target->Send(ticket.first, 1, ticket.second);
}

int IOCPFramework::SendMoveEntity(Session* target, PID cid, float nx, float ny, MOVE_TYPES dir) const
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
	return clientsPool[index].load(std::memory_order_relaxed);
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

SOCKET IOCPFramework::CreateSocket() const volatile
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED);
}
