#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <unordered_map>
#include <queue>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")
using namespace std;

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

constexpr int SIGHT = 20;
constexpr int PLAYERS_ID_BEGIN = 10000;
constexpr int NPC_NUMBER = 10000;

enum COMP_TYPE
{
	OP_ACCEPT, OP_RECV, OP_SEND, COMP_NPC_MOVE
};
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };
enum ENTITY_TYPE { NONE, PLAYER, NPC };
enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK };

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int caller = -1;

	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

class TIMER
{
public:
	int entitiy_id; // 누가 발생시키는가
	EVENT_TYPE type;
	chrono::system_clock::time_point adapt_time;
};

priority_queue<TIMER> myEvents;

void add_timer(int entity_id, int ms, EVENT_TYPE type, int target_id)
{
	TIMER timer;

	timer.entitiy_id = entity_id;
	timer.adapt_time = chrono::system_clock::now() + chrono::milliseconds(ms);
	timer.type = type;
}

class NPC
{
public:
	atomic_int _id = 0;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;

	SOCKET _socket;
	lua_State* lua;

	void SendChatMsg(const WCHAR* chat, const size_t length)
	{
		SC_CHAT_PACKET p{};
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;

		if (p.caption) delete p.caption;
		p.caption = new WCHAR[length];
		lstrcpyn(p.caption, chat, length);

		do_send(&p);
	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

protected:
	OVER_EXP _recv_over;
};

class SESSION : public NPC
{
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;

		// 처음에 좌표를 과하게 몰아놓으면 과부하가 걸린다.
		x = int(rand() / RAND_MAX * W_WIDTH);
		y = int(rand() / RAND_MAX * W_HEIGHT);


		_name[0] = 0;
		_s_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);

	mutex	_sl;
	SESSION_STATE _s_state;
	unordered_set<int> view_list;
};

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	do_send(&p);
}



mutex client_lock;
array<SESSION, MAX_USER> clients;
HANDLE g_h_iocp;
SOCKET g_s_socket;
vector<SESSION> global_view_list;
void disconnect(int c_id);
int self_id = 0;

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		clients[i]._sl.lock();
		if (clients[i]._s_state == ST_FREE)
		{
			clients[i]._s_state = ST_ACCEPTED;
			clients[i]._sl.unlock();
			return i;
		}
		clients[i]._sl.unlock();
	}
	return -1;
}

int distance(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

// 일정 시간마다 NPC 인공지능 작동 (heartbeat)
void do_ai_version_1() {}

void move_npc(int npc_id)
{
	global_view_list.clear();

	auto& npc = clients[npc_id];
	short& nx = npc.x;
	short& ny = npc.y;

	unordered_set<int> pos_set{};
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (npc._s_state != ST_INGAME)
		{
			continue;
		}

		if (distance(npc_id, i) <= PLAYERS_ID_BEGIN)
		{
			global_view_list.push_back(npc);
		}

		// 플레이어를 추가하는 패킷을 클라이언트에 전송
		// SC_ADD_PLAYER;
		for (int p_id = 0; p_id < MAX_USER; ++p_id)
		{
			auto& target_session = clients[p_id];
			auto& p_x = target_session.x;
			auto& p_y = target_session.y;

			if (distance(nx, ny, p_x, p_y) < SIGHT)
			{
				SendAddPlayer(target_session, npc_id);
			}
		}
	}
}

// 'AddPlayer' 말고 'AddEntity', 'AddViewInstance' 같은 이름을 써라
// 일부러 함수 이름 안 바꿈
void SendAddPlayer(SESSION& session, int id)
{
	client_lock.lock();

	SC_ADD_PLAYER_PACKET packet{};
	packet.id = id;
	session.do_send(&packet);

	client_lock.unlock();
}

void SendViewList(SESSION& session)
{
	// 시야 목록의 수정, 송신은 반드시 락을 걸고 하라.
	client_lock.lock();

	client_lock.unlock();
}

void process_packet(int c_id, char* packet)
{
	auto& this_session = clients[c_id];

	switch (packet[1])
	{
		case CS_LOGIN:
		{
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
			this_session._sl.lock();
			if (this_session._s_state == ST_FREE)
			{
				this_session._sl.unlock();
				break;
			}
			if (this_session._s_state == ST_INGAME)
			{
				this_session._sl.unlock();
				disconnect(c_id);
				break;
			}

			strcpy_s(this_session._name, p->name);
			this_session.send_login_info_packet();
			this_session._s_state = ST_INGAME;
			this_session._sl.unlock();

			for (auto& pl : clients)
			{
				if (pl._id == c_id) continue;
				if (ST_INGAME != pl._s_state)
				{
					continue;
				}

				const int dist = distance(this_session.x, this_session.y
					, pl.x, pl.y);

				if (dist < SIGHT)
				{
					this_session.view_list.emplace(pl._id);

					pl._sl.lock();

					SC_ADD_PLAYER_PACKET add_packet;
					add_packet.id = c_id;
					strcpy_s(add_packet.name, p->name);

					add_packet.size = sizeof(add_packet);
					add_packet.type = SC_ADD_PLAYER;
					add_packet.x = this_session.x;
					add_packet.y = this_session.y;

					pl.do_send(&add_packet);

					pl._sl.unlock();
				}
			}

			for (auto& pl : clients)
			{
				if (pl._id == c_id) continue;
				if (ST_INGAME != pl._s_state) continue;

				pl._sl.lock();

				lock_guard<mutex> aa{ pl._sl };
				const int dist = distance(this_session.x, this_session.y
					, pl.x, pl.y);

				if (dist < SIGHT)
				{
					pl.view_list.emplace(pl._id);

					SC_ADD_PLAYER_PACKET add_packet{};
					add_packet.id = pl._id;

					strcpy_s(add_packet.name, pl._name);
					add_packet.size = sizeof(add_packet);
					add_packet.type = SC_ADD_PLAYER;
					add_packet.x = pl.x;
					add_packet.y = pl.y;

					this_session.do_send(&add_packet);
				}
			}

			break;
		}
		case CS_MOVE:
		{
			CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
			auto& cx = this_session.x;
			auto& cy = this_session.y;

			short x = cx;
			short y = cy;
			switch (p->direction)
			{
				case 0: if (y > 0) y--; break;
				case 1: if (y < W_HEIGHT - 1) y++; break;
				case 2: if (x > 0) x--; break;
				case 3: if (x < W_WIDTH - 1) x++; break;
			}

			cx = x;
			cy = y;
			for (auto& pl : clients)
			{
				const int dist = distance(this_session.x, this_session.y
					, pl.x, pl.y);

				if (dist < SIGHT)
				{
					lock_guard<mutex> aa{ pl._sl };

					if (!pl.view_list.contains(c_id))
					{
						if (ST_INGAME == pl._s_state)
						{
							pl.send_move_packet(c_id);
						}

						pl.view_list.insert(c_id);
					}
				}
			}
			break;
		}
	}
}

void disconnect(int c_id)
{
	clients[c_id]._sl.lock();
	if (clients[c_id]._s_state == ST_FREE)
	{
		clients[c_id]._sl.unlock();
		return;
	}
	closesocket(clients[c_id]._socket);
	clients[c_id]._s_state = ST_FREE;
	clients[c_id]._sl.unlock();

	for (auto& pl : clients)
	{
		if (pl._id == c_id) continue;
		pl._sl.lock();
		if (pl._s_state != ST_INGAME)
		{
			pl._sl.unlock();
			continue;
		}
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);
		pl._sl.unlock();
	}
}

void do_worker()
{
	while (true)
	{
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_bytes, &key, &over, INFINITE);

		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret)
		{
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else
			{
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type)
		{
			case OP_ACCEPT:
			{
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1)
				{
					clients[client_id].x = short((rand() / RAND_MAX) * W_WIDTH);
					clients[client_id].y = short((rand() / RAND_MAX) * W_HEIGHT);

					clients[client_id]._id = client_id;
					clients[client_id]._name[0] = 0;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = c_socket;

					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
						g_h_iocp, client_id, 0);
					clients[client_id].do_recv();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else
				{
					cout << "Max user exceeded.\n";
				}

				ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
				ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);

				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_s_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
				break;
			}

			case OP_RECV:
			{
				if (0 == num_bytes) disconnect(key);
				int remain_data = num_bytes + clients[key]._prev_remain;
				char* p = ex_over->_send_buf;

				while (remain_data > 0)
				{
					int packet_size = p[0];
					if (packet_size <= remain_data)
					{
						process_packet(static_cast<int>(key), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}

				clients[key]._prev_remain = remain_data;
				if (remain_data > 0)
				{
					memcpy(ex_over->_send_buf, p, remain_data);
				}
				clients[key].do_recv();
				break;
			}
			case OP_SEND:
			if (0 == num_bytes) disconnect(key);
			delete ex_over;
			break;
		}
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	listen(g_s_socket, SOMAXCONN);

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 9999, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	OVER_EXP a_over;
	a_over._comp_type = OP_ACCEPT;
	a_over._wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	AcceptEx(g_s_socket, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	vector <thread> worker_threads;

	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(do_worker);

	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}
