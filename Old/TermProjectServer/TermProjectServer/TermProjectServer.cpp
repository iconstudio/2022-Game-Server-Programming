#include "pch.hpp"
#include "TermProjectServer.hpp"
#include "Framework.hpp"

HANDLE serverPort;
SOCKET serverListener;
const ULONG_PTR serverID = 30000;
const DWORD threadsCount = 6;
char acceptBuffer[512]{};
DWORD acceptByte = 0;
atomic<SOCKET> acceptNewbie = NULL;

concurrent_vector<Timeline> timer_queue{};
std::mutex timer_l;

array<shared_atomic<Session>, MAX_USER + MAX_NPC> clients;
extern size_t numberClients = 0;

constexpr int SIGHT_RANGE = 7;

void add_timer(PID obj_id, int act_time, EVENT_TYPE e_type, PID target_id)
{
	using namespace std::chrono;

	Timeline ev{};

	ev.act_time = system_clock::now() + milliseconds(act_time);
	ev.object_id = obj_id;
	ev.ev = e_type;
	ev.target_id = target_id;

	timer_queue.push_back(ev);
}

void process_packet(PID client_id, const Packet* packet)
{
	auto session = AcquireSession(client_id);

	const auto pk_type = packet->myType;

	switch (pk_type)
	{
		case PACKET_TYPES::CS_LOGIN:
		{
			const auto p = reinterpret_cast<const CS_LOGIN_PACKET*>(packet);

			auto newbie_status = session->AcquireStatus();

			switch (newbie_status)
			{
				case ST_FREE:
				{}
				break;

				case ST_INGAME:
				{
					Disconnect(client_id);
				}
				break;

				case ST_ACCEPTED:
				{
					char temp_nickname[NAME_SIZE]{};
					if (strcmp("TEMP", p->myNickname) == 0)
					{
					//	sprintf_s(temp_nickname, "P%zd", client_id);
					}
					else if (0 < strlen(p->myNickname))
					{
					//	strcpy_s(temp_nickname, p->myNickname);
					}
					//strcpy_s(session->myNickname, temp_nickname);

					session->SetStatus(ST_INGAME);

					auto avatar = make_shared<GameObject>();
					avatar->x = rand() % W_WIDTH;
					avatar->y = rand() % W_HEIGHT;
					session->SetAvatar(avatar);

					session->send_login_info_packet(numberClients);

					// 원래 있던 플레이어들에게 새로 접속한 플레이어의 시야 정보 전송
					for (auto i = ORDER_BEGIN_USER; i < ORDER_END_USER; i++)
					{
						if (i == client_id)
						{
							continue;
						}

						auto other = AcquireSession(i);
						auto other_statsus = other->AcquireStatus();

						if (ST_INGAME != other_statsus)
						{
							other->ReleseStatus(other_statsus);
							ReleaseSession(i, other);

							continue;
						}

						if (GetGridDistance(client_id, i) <= SIGHT_RANGE)
						{
							std::unique_lock guard(other->cvSight);
							other->view_list.insert(client_id);
							guard.unlock();

							other->send_add_object(client_id, session);
						}

						other->ReleseStatus(other_statsus);
						ReleaseSession(i, other);
					}

					// 접속한 플레이어에게 시야 전송
					for (auto i = 0; i < ORDER_END_ALL; i++)
					{
						if (i == client_id)
						{
							continue;
						}

						auto other = AcquireSession(i);
						auto other_statsus = other->AcquireStatus();

						if (ST_INGAME != other_statsus)
						{
							other->ReleseStatus(other_statsus);
							ReleaseSession(i, other);

							continue;
						}

						//if (GetGridDistance(i, client_id) <= SIGHT_RANGE)
						if (0 != other->view_list.count(client_id))
						{
							std::unique_lock guard(session->cvSight);
							session->view_list.insert(other->myID);
							guard.unlock();

							session->send_add_object(other->myID, other);
						}

						other->ReleseStatus(other_statsus);
						ReleaseSession(i, other);
					}
				}
				break;

				default:
				{
					throw "패킷 오류!";
				}
				break;
			}

			session->ReleseStatus(newbie_status);
		}
		break;

		case PACKET_TYPES::CS_MOVE:
		{
			const auto p = reinterpret_cast<const CS_MOVE_PACKET*>(packet);
			auto avatar = session->AcquireAvatar();

			if (!avatar)
			{
				ReleaseSession(client_id, session);
				std::cout << "MOVE: 클라이언트 " << client_id << "의 아바타를 찾을 수 없습니다!\n";
				return;
			}

			auto& cl_x = avatar->x;
			auto& cl_y = avatar->y;

			session->cvSight.lock();
			auto old_sight_list = session->view_list;
			session->cvSight.unlock();

			switch (p->direction)
			{
				case MOVE_TYPES::LEFT:
				{
					if (cl_x > 0)
						cl_x--;
				}
				break;

				case MOVE_TYPES::RIGHT:
				{
					if (cl_x < W_WIDTH - 1)
						cl_x++;
				}
				break;

				case MOVE_TYPES::UP:
				{
					if (cl_y > 0)
						cl_y--;
				}
				break;

				case MOVE_TYPES::DOWN:
				{
					if (cl_y < W_HEIGHT - 1)
						cl_y++;
				}
				break;
			}

			std::unordered_set<PID> new_sight_list{};

			for (auto pid = ORDER_BEGIN_USER; pid < ORDER_END_USER; pid++)
			{
				auto other = AcquireSession(pid);
				auto& other_id = other->myID;

				if (other->myID == client_id)
				{
					continue;
				}
				if (ST_INGAME != other->myStatus)
				{
					continue;
				}

				if (SIGHT_RANGE < GetGridDistance(client_id, other_id))
				{
					ReleaseSession(pid, other);
					continue;
				}

				new_sight_list.insert(other_id);
				ReleaseSession(other_id, other);
			}
			session->send_move_packet(client_id, session, p->client_time);

			auto& my_view_list = session->view_list;
			for (const auto& instance_id : new_sight_list)
			{
				std::unique_lock guard(session->cvSight);

				auto other = AcquireSession(instance_id);

				if (0 == my_view_list.count(instance_id))
				{
					my_view_list.insert(instance_id);
					guard.unlock();

					session->send_add_object(instance_id, other);
				}
				else
				{
					guard.unlock();

					session->send_move_packet(instance_id, other, 0);
				}

				if (IsPlayer(instance_id))
				{
					std::unique_lock other_guard(other->cvSight);

					auto& other_view_list = other->view_list;
					if (0 == other_view_list.count(client_id))
					{
						other_view_list.insert(client_id);
						other_guard.unlock();

						other->send_add_object(client_id, session);
					}
					else
					{
						other_guard.unlock();

						other->send_move_packet(client_id, session, 0);
					}
				}

				ReleaseSession(instance_id, other);
			}

			for (auto instance_id : old_sight_list)
			{
				if (0 != new_sight_list.count(instance_id))
				{
					continue;
				}

				std::unique_lock guard(session->cvSight);

				auto other = AcquireSession(instance_id);

				if (0 != my_view_list.count(instance_id))
				{
					my_view_list.erase(instance_id);
					guard.unlock();

					session->send_remove_object(instance_id);
				}
				else
				{
					guard.unlock();
				}

				if (IsPlayer(instance_id))
				{
					std::unique_lock other_guard(other->cvSight);

					auto& other_view_list = other->view_list;
					if (0 != other_view_list.count(client_id))
					{
						other_view_list.erase(client_id);
						other_guard.unlock();

						other->send_remove_object(client_id);
					}
					else
					{
						other_guard.unlock();
					}
				}

				ReleaseSession(instance_id, other);
			}
		}
		break;

		case PACKET_TYPES::CS_ATTACK:
		{
		}
		break;

		case PACKET_TYPES::CS_ATTACK_NONTARGET:
		{
		}
		break;

		case PACKET_TYPES::CS_SKILL_1:
		{
		}
		break;

		case PACKET_TYPES::CS_SKILL_2:
		{
		}
		break;

		case PACKET_TYPES::CS_CHAT:
		{
		}
		break;
	}

	ReleaseSession(client_id, session);
}

void do_ai_ver_1()
{
	while (true)
	{
		auto start_t = std::chrono::system_clock::now();

		for (auto i = ORDER_BEGIN_NPC; i < ORDER_END_NPC; i++)
		{
			const auto npc = GetSession(i);

			auto& behave_time = npc->next_move_time;
			if (behave_time < start_t)
			{
				BehaveNPC(i);

				behave_time = start_t + std::chrono::seconds(1);
			}
		}
	}
}

void do_ai_ver_heat_beat()
{
	while (true)
	{
		const auto start_t = std::chrono::system_clock::now();

		for (auto i = ORDER_BEGIN_NPC; i < ORDER_END_NPC; i++)
		{
			BehaveNPC(i);
		}

		const auto end_t = std::chrono::system_clock::now();
		const auto ai_t = end_t - start_t;

		std::cout << "AI time : " << std::chrono::duration_cast<std::chrono::milliseconds>(ai_t).count();

		std::cout << "ms\n";
		std::this_thread::sleep_until(start_t + std::chrono::seconds(1));
	}
}

void do_timer()
{

}

int main()
{
	Awake();
	Start();
	Update();
	Release();
}

void Awake()
{
	WSADATA WSAData{};
	int result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (SOCKET_ERROR == result)
	{
		throw "WSAStartup()";
	}

	serverListener = MakeSocket();
	if (INVALID_SOCKET == serverListener)
	{
		throw "WSASocket(listener)";
	}

	SOCKADDR_IN server_addr{};
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);

	constexpr int address_size = sizeof(SOCKADDR_IN);

	result = bind(serverListener, reinterpret_cast<sockaddr*>(&server_addr), address_size);
	if (SOCKET_ERROR == result)
	{
		throw "bind()";
	}

	serverPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadsCount);
	if (NULL == serverPort)
	{
		throw "CreateIoCompletionPort(INVALID_HANDLE_VALUE)";
	}

	auto handle = reinterpret_cast<HANDLE>(serverListener);
	CreateIoCompletionPort(handle, serverPort, serverID, 0);

	std::cout << "서버 초기화 중...\n";

	BuildSessions();
	BuildPlayers();
	BuildNPCs();
}

void BuildSessions()
{
	for (PID i = 0; i < MAX_NPC + MAX_USER; ++i)
	{
		auto session = AcquireSession(i);

		auto handle = make_shared<Session>();
		handle->myID = i;

		ReleaseSession(i, handle);
	}
}

void BuildPlayers()
{}

void BuildNPCs()
{
	constexpr auto order_npc_begin = ORDER_BEGIN_NPC;
	constexpr auto order_npc_end = order_npc_begin + MAX_NPC;

	for (auto i = order_npc_begin; i < order_npc_end; ++i)
	{
		const PID npc_id = i + MAX_USER;

		auto npc = AcquireSession(i);

		auto npc_avatar = npc->AcquireAvatar();
		npc_avatar = make_shared<GameObject>();
		npc_avatar->x = 4;
		npc_avatar->y = 4;
		npc->ReleseAvatar(npc_avatar);

		npc->SetStatus(ST_INGAME);
		sprintf_s(npc->myNickname, "M-%d", npc_id);

		ReleaseSession(i, npc);
	}
}

void Start()
{
	int result = listen(serverListener, SOMAXCONN);
	if (SOCKET_ERROR == result)
	{
		throw "listen()";
	}

	acceptNewbie = MakeSocket();
	if (NULL == acceptNewbie)
	{
		throw "MakeSocket(first_socket)";
	}

	auto a_over = new Asynchron{};
	a_over->myOperation = OP_ACCEPT;

	Listen(&a_over->_over);

	std::cout << "서버 준비 중...\n";

	std::vector<std::jthread> worker_threads{};
	for (int i = 0; i < 6; ++i)
	{
		worker_threads.emplace_back(ConcurrentWorker);
	}

	std::jthread ai_thread{ do_ai_ver_heat_beat };

	std::cout << "서버 시작\n";
}

void Listen(LPWSAOVERLAPPED overlapped)
{
	const int address_size = sizeof(SOCKADDR_IN);
	int result = AcceptEx(serverListener, acceptNewbie
		, acceptBuffer, 0
		, address_size + 16, address_size + 16
		, &acceptByte
		, overlapped);
	if (FALSE == result)
	{
		auto error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			ZeroMemory(overlapped, sizeof(WSAOVERLAPPED));

			throw "Accept 오류!";
		}
	}
}

void Update()
{
	while (true)
	{

	}
}

void ConcurrentWorker()
{
	DWORD portBytes = 0;
	ULONG_PTR portKey = 0;
	WSAOVERLAPPED* portOverlap = nullptr;

	BOOL result;
	while (true)
	{
		result = GetQueuedCompletionStatus(serverPort, &portBytes, &portKey, &portOverlap, INFINITE);

		auto asynchron = reinterpret_cast<Asynchron*>(portOverlap);
		
		if (FALSE == result)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				if (asynchron->myOperation == OP_ACCEPT)
				{
					std::cout << "Accept 오류!\n";

					asynchron->Clear();
				}
				else
				{
					const PID client_id = static_cast<PID>(portKey);

					std::cout << "GQCS Error on client[" << client_id << "]\n";

					Disconnect(client_id);

					if (asynchron->myOperation == OP_SEND)
					{
						delete asynchron;
					}
					else
					{
						asynchron->Clear();
					}
				}
			}
		}
		else
		{
			switch (asynchron->myOperation)
			{
				case OP_ACCEPT:
				{
					ProceedAccept(asynchron);
				}
				break;

				case OP_RECV:
				{
					const PID client_id = static_cast<PID>(portKey);
					ProceedRecv(client_id, asynchron, portBytes);
				}
				break;

				case OP_SEND:
				{
					const PID client_id = static_cast<PID>(portKey);
					ProceedSend(client_id, asynchron, portBytes);

					delete asynchron;
				}
				break;
			}
		}
	}
}

void ProceedAccept(Asynchron* asynchron)
{
	const auto newbie_socket = acceptNewbie.load(std::memory_order_acquire);
	if (NULL == newbie_socket)
	{
		acceptNewbie.store(newbie_socket, std::memory_order_release);
		throw "ProceedAccept(newbie_socket) 오류!";
	}

	const auto newbie_id = MakeNewbieID();

	if (PID(-1) != newbie_id)
	{
		auto newbie = AcquireSession(newbie_id);

		auto newbie_status = newbie->AcquireStatus();

		auto newbie_avatar = newbie->AcquireAvatar();
		newbie_avatar = make_shared<GameObject>();
		newbie_avatar->x = 4;
		newbie_avatar->y = 4;
		newbie->ReleseAvatar(newbie_avatar);

		newbie->SetID(newbie_id);
		newbie->SetSocket(newbie_socket);

		newbie->ReleseStatus(newbie_status);
		ReleaseSession(newbie_id, newbie);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(newbie_socket), serverPort, newbie_id, 0);

		newbie->do_recv();

		acceptNewbie.store(MakeSocket(), std::memory_order_release);
	}
	else
	{
		acceptNewbie.store(newbie_socket, std::memory_order_release);

		std::cout << "Max user exceeded.\n";
	}
	asynchron->Clear();

	Listen(&asynchron->_over);
}

void ProceedRecv(const PID pid, Asynchron* asynchron, const DWORD recv_bytes)
{
	if (0 == recv_bytes)
	{
		std::cout << "클라이언트 " << pid << "가 수신에서 0을 받음.\n";
		Disconnect(pid);

		return;
	}

	const char* raw_packet = asynchron->_send_buf;
	const Packet* raw_view = reinterpret_cast<const Packet*>(raw_packet);

	auto session = AcquireSession(pid);
	auto& saved_bytes = session->recvBytes;

	saved_bytes += recv_bytes;
	while (0 < saved_bytes)
	{
		// int?
		const unsigned char packet_size = raw_view->mySize;
			//raw_packet[0];
		if (packet_size <= saved_bytes)
		{
			process_packet(pid, raw_view);

			raw_packet = raw_packet + packet_size;
			saved_bytes -= packet_size;
		}
		else
		{
			break;
		}
	}

	if (0 < saved_bytes)
	{
		memcpy(asynchron->_send_buf, raw_packet, saved_bytes);
	}

	ReleaseSession(pid, session);

	session->do_recv();
}

void ProceedSend(const PID pid, Asynchron* asynchron, const DWORD sent_bytes)
{
	if (0 == sent_bytes)
	{
		std::cout << "클라이언트 " << pid << "가 송신에 0을 줌.\n";
		Disconnect(pid);

		return;
	}
}

void Release()
{
	closesocket(serverListener);
	WSACleanup();
}

int GetGridDistance(PID user_id1, PID user_id2)
{
	const auto& user1 = GetSession(user_id1);
	const auto& user2 = GetSession(user_id2);

	const auto avatar1 = user1->GetAvatar();
	const auto avatar2 = user2->GetAvatar();

	return static_cast<int>(abs(avatar1->x - avatar2->x) + abs(avatar1->y - avatar2->y));
}

void BehaveNPC(PID npc_id)
{
	auto npc = AcquireSession(npc_id);
	auto npc_avatar = npc->AcquireAvatar();

	auto& npc_x = npc_avatar->x;
	auto& npc_y = npc_avatar->y;

	std::unordered_set<PID> old_view_list{};
	for (auto i = ORDER_BEGIN_USER; i < ORDER_END_USER; ++i)
	{
		const auto session = GetSession(i);

		if (session->myStatus != ST_INGAME)
		{
			continue;
		}

		if (GetGridDistance(npc_id, i) <= SIGHT_RANGE)
		{
			old_view_list.insert(i);
		}
	}

	switch (rand() % 5)
	{
		case 0: break;
		case 1: if (npc_y > 0) npc_y--; break;
		case 2: if (npc_y < W_HEIGHT - 1) npc_y++; break;
		case 3: if (npc_x > 0) npc_x--; break;
		case 4: if (npc_x < W_WIDTH - 1) npc_x++; break;
	}

	std::unordered_set<PID> new_view_list{};
	for (auto i = ORDER_BEGIN_USER; i < ORDER_END_USER; ++i)
	{
		const auto session = GetSession(i);

		if (session->myStatus != ST_INGAME)
		{
			continue;
		}

		if (GetGridDistance(npc_id, i) <= SIGHT_RANGE)
		{
			new_view_list.insert(i);
		}
	}

	// 새로운 시야는 전송
	for (const auto session_id : new_view_list)
	{
		const auto session = AcquireSession(session_id);
		auto& view_list = session->view_list;

		std::unique_lock guard(session->cvSight);

		auto oit = old_view_list.find(session_id);
		if (oit != old_view_list.end())
		{
			old_view_list.erase(oit);
		}

		if (0 == view_list.count(npc_id))
		{
			view_list.insert(npc_id);
			guard.unlock();

			session->send_add_object(npc_id, npc);
		}
		else
		{
			guard.unlock();

			session->send_move_packet(npc_id, npc, 0);
		}

		ReleaseSession(session_id, session);
	}

	// 예전 시야는 삭제
	for (auto session_id : old_view_list)
	{
		if (0 == new_view_list.count(session_id))
		{
			const auto session = AcquireSession(session_id);
			auto& view_list = session->view_list;

			std::unique_lock guard(session->cvSight);

			if (0 != view_list.count(npc_id))
			{
				view_list.erase(npc_id);
				guard.unlock();

				session->send_remove_object(npc_id);
			}
			else
			{
				guard.unlock();
			}

			ReleaseSession(session_id, session);
		}
	}

	npc->ReleseAvatar(npc_avatar);
	ReleaseSession(npc_id, npc);
}

SOCKET MakeSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

PID MakeNewbieID()
{
	for (auto i = ORDER_BEGIN_USER; i < ORDER_END_USER; i++)
	{
		auto session = AcquireSession(i);

		auto state = session->AcquireStatus();

		if (ST_FREE == state)
		{
			state = ST_ACCEPTED;

			session->ReleseStatus(state);
			ReleaseSession(i, session);

			return i;
		}

		session->ReleseStatus(state);
		ReleaseSession(i, session);
	}

	return PID(-1);
}

void Disconnect(const PID who)
{
	if (!IsPlayer(who))
	{
		throw "Disconnect: 잘못된 ID 참조!";
	}

	auto session = AcquireSession(who);
	auto status = session->AcquireStatus();

	if (status == ST_FREE)
	{
		session->ReleseStatus(status);
		ReleaseSession(who, session);

		return;
	}

	for (auto other_id = ORDER_BEGIN_USER; other_id < ORDER_END_USER; other_id++)
	{
		if (other_id == who)
		{
			continue;
		}

		auto other = AcquireSession(other_id);
		auto other_status = other->AcquireStatus();

		if (ST_INGAME != other_status)
		{
			other->ReleseStatus(other_status);
			ReleaseSession(other_id, other);

			continue;
		}

		std::unique_lock sight_guard(other->cvSight);

		auto& view_list = other->view_list;
		if (0 != view_list.count(who))
		{
			view_list.erase(who);
			sight_guard.unlock();

			SC_REMOVE_OBJECT_PACKET p{ who };

			other->do_send(&p, static_cast<size_t>(p.mySize));
		}
		else
		{
			//sight_guard.unlock();
		}
	}

	closesocket(session->mySocket);
	status = ST_FREE;

	session->ReleseStatus(status);
	ReleaseSession(who, session);
}

shared_ptr<Session> AcquireSession(PID index)
{
	return clients[index].load(std::memory_order_acquire);
}

void ReleaseSession(PID index, shared_ptr<Session> handle)
{
	clients[index].store(handle, std::memory_order_release);
}

shared_ptr<Session> GetSession(PID index)
{
	return clients[index].load(std::memory_order_relaxed);
}
