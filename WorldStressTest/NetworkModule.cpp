#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32.lib")
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <array>
#include <memory>

#include "Commons.hpp"

using namespace std;
using namespace chrono;

const static int MAX_TEST = USERS_MAX;
const static int MAX_CLIENTS = ENTITIES_MAX_NUMBER;

const static int INVALID_ID = -1;
const static int MAX_PACKET_SIZE = 255;
const static int MAX_BUFF_SIZE = 255;

extern HWND hWnd;
HANDLE g_hiocp;

high_resolution_clock::time_point last_connect_time;

struct OverlappedEx
{
	WSAOVERLAPPED over;
	WSABUF wsabuf;

	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	unsigned int recv_prev = 0;

	OVERLAP_OPS event_type;
	int event_target;
};

struct CLIENT
{
	int id;
	int x;
	int y;
	atomic_bool connected;

	SOCKET client_socket;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_packet_data;
	int curr_packet_size;
	high_resolution_clock::time_point last_move_time;
};

array<PID, MAX_CLIENTS> client_map;
array<CLIENT, MAX_CLIENTS> everySessions;

atomic_int num_connections;
atomic_int client_to_close;
atomic_int active_clients;

int			global_delay;				// ms단위, 1000이 넘으면 클라이언트 증가 종료

vector <thread*> worker_threads;
thread test_thread;

float point_cloud[MAX_TEST * 2];

// 나중에 NPC까지 추가 확장 용
struct ALIEN
{
	int id;
	int x, y;
	int visible_count;
};

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;

	MessageBox(hWnd, lpMsgBuf, L"ERROR", 0);
	LocalFree(lpMsgBuf);
	// while (true);
}

void DisconnectClient(PID ci)
{
	if (IsPlayer(ci))
	{
		bool status = true;

		if (true == atomic_compare_exchange_strong(&everySessions[ci].connected, &status, false))
		{
			closesocket(everySessions[ci].client_socket);
			active_clients--;
		}

		cout << "Client [" << ci << "] Disconnected!\n";
	}
}

void SendPacket(PID cl, const void* packet)
{
	const auto raw_packet_view = reinterpret_cast<const Packet*>(packet);
	const auto psize = raw_packet_view->Size;
	const auto ptype = raw_packet_view->Type;

	OverlappedEx* over = new OverlappedEx{};
	over->event_type = OVERLAP_OPS::SEND;

	memcpy(over->IOCP_buf, packet, psize);
	ZeroMemory(&over->over, sizeof(over->over));

	over->wsabuf.buf = reinterpret_cast<CHAR*>(over->IOCP_buf);
	over->wsabuf.len = psize;

	int ret = WSASend(everySessions[cl].client_socket, &over->wsabuf, 1, NULL, 0,
		&over->over, NULL);
	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket:", err_no);
	}

	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void ProcessPacket(PID ci, const unsigned char* packet)
{
	const auto raw_packet = const_cast<unsigned char*>(packet);
	const auto raw_packet_view = reinterpret_cast<const Packet*>(raw_packet);

	const auto ptype = raw_packet_view->Type;

	switch (ptype)
	{
		case PACKET_TYPES::SC_MOVE_OBJ:
		{
			const auto move_packet = reinterpret_cast<SCPacketMoveCharacter*>(raw_packet);

			if (IsPlayer(move_packet->playerID))
			{
				const PID my_local_id = move_packet->playerID - CLIENTS_ORDER_BEGIN;
				int my_id = client_map[my_local_id];

				if (-1 != my_id)
				{
					everySessions[my_id].x = move_packet->x;
					everySessions[my_id].y = move_packet->y;
				}

				if (ci == my_id)
				{
					if (0 != move_packet->move_time)
					{
						auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - move_packet->move_time;

						if (global_delay < d_ms) global_delay++;
						else if (global_delay > d_ms) global_delay--;
					}
				}
			}
		}
		break;

		case PACKET_TYPES::SC_SIGNIN_FAILED:
		case PACKET_TYPES::SC_SIGNOUT:
		{}
		break;

		case PACKET_TYPES::SC_STAT_OBJ:
		case PACKET_TYPES::SC_SIGNUP:
		case PACKET_TYPES::SC_CREATE_PLAYER:
		case PACKET_TYPES::SC_DISAPPEAR_OBJ:
		{}
		break;

		case PACKET_TYPES::SC_APPEAR_OBJ:
		{
			const auto my_id = ci;

			everySessions[ci].connected = true;
			active_clients++;

			const auto login_packet = reinterpret_cast<SCPacketAppearEntity*>(raw_packet);

			if (IsPlayer(my_id))
			{
				const PID my_local_id = login_packet->playerID - CLIENTS_ORDER_BEGIN;

				client_map[my_local_id] = my_id;
			}

			everySessions[my_id].id = login_packet->playerID;
			everySessions[my_id].x = login_packet->x;
			everySessions[my_id].y = login_packet->y;

			//cs_packet_teleport t_packet;
			//t_packet.size = sizeof(t_packet);
			//t_packet.type = CS_TELEPORT;
			//SendPacket(my_id, &t_packet);
		}
		break;

		default:
		{
			MessageBox(hWnd, L"Unknown Packet Type", L"ERROR", 0);
			while (true);
		}
	}
}

void Worker_Thread()
{
	while (true)
	{
		DWORD io_size;
		unsigned long long ci = 0;
		OverlappedEx* over = nullptr;

		BOOL ret = GetQueuedCompletionStatus(g_hiocp, &io_size, &ci,
			reinterpret_cast<LPWSAOVERLAPPED*>(&over), INFINITE);
		std::cout << "GQCS :";

		PID client_id = static_cast<PID>(ci);
		if (FALSE == ret)
		{
			int err_no = WSAGetLastError();

			if (64 == err_no)
			{
				DisconnectClient(client_id);
			}
			else
			{
				 error_display("GQCS : ", WSAGetLastError());
				DisconnectClient(client_id);
			}

			if (OVERLAP_OPS::SEND == over->event_type) delete over;
		}

		if (0 == io_size)
		{
			DisconnectClient(client_id);
			continue;
		}

		if (OVERLAP_OPS::RECV == over->event_type)
		{
			std::cout << "RECV from Client :" << ci;
			std::cout << "  IO_SIZE : " << io_size << std::endl;

			unsigned char* buf = everySessions[client_id].recv_over.IOCP_buf;
			unsigned proceed_size = everySessions[client_id].curr_packet_size;
			unsigned pr_size = everySessions[client_id].prev_packet_data;

			while (io_size > 0)
			{
				const auto packet_view = reinterpret_cast<Packet*>(buf);
				if (0 == proceed_size)
				{
					proceed_size = packet_view->Size;
				}

				if (proceed_size <= io_size + pr_size)
				{
					// 지금 패킷 완성 가능
					unsigned char packet[MAX_PACKET_SIZE]{};
					memcpy(packet, everySessions[client_id].packet_buf, pr_size);
					memcpy(packet + pr_size, buf, static_cast<size_t>(proceed_size - pr_size));

					ProcessPacket(client_id, packet);

					io_size -= proceed_size - pr_size;
					buf += proceed_size - pr_size;
					proceed_size = 0; pr_size = 0;
				}
				else
				{
					memcpy(everySessions[client_id].packet_buf + pr_size, buf, io_size);

					pr_size += io_size;
					io_size = 0;
				}
			}

			everySessions[client_id].curr_packet_size = proceed_size;
			everySessions[client_id].prev_packet_data = pr_size;

			DWORD recv_flag = 0;
			int ret = WSARecv(everySessions[client_id].client_socket,
				&everySessions[client_id].recv_over.wsabuf, 1,
				NULL, &recv_flag, &everySessions[client_id].recv_over.over, NULL);
			if (SOCKET_ERROR == ret)
			{
				int err_no = WSAGetLastError();
				if (err_no != WSA_IO_PENDING)
				{
					//error_display("RECV ERROR", err_no);
					DisconnectClient(client_id);
				}
			}
		}
		else if (OVERLAP_OPS::SEND == over->event_type)
		{
			if (io_size != over->wsabuf.len)
			{
				// std::cout << "Send Incomplete Error!\n";
				DisconnectClient(client_id);
			}
			delete over;
		}
		else if (OVERLAP_OPS::ENTITY_MOVE == over->event_type)
		{
			// Not Implemented Yet
			delete over;
		}
		else
		{
			std::cout << "Unknown GQCS event!\n";
			while (true);
		}
	}
}

constexpr int DELAY_LIMIT = 100;
constexpr int DELAY_LIMIT2 = 150;
constexpr int ACCEPT_DELY = 50;

void Adjust_Number_Of_Client()
{
	static int delay_multiplier = 1;
	static int max_limit = MAXINT;
	static bool increasing = true;

	if (active_clients >= MAX_TEST) return;
	if (num_connections >= MAX_CLIENTS) return;

	auto duration = high_resolution_clock::now() - last_connect_time;
	if (ACCEPT_DELY * delay_multiplier > duration_cast<milliseconds>(duration).count()) return;

	int t_delay = global_delay;
	if (DELAY_LIMIT2 < t_delay)
	{
		if (true == increasing)
		{
			max_limit = active_clients;
			increasing = false;
		}

		if (100 > active_clients) return;

		if (ACCEPT_DELY * 10 > duration_cast<milliseconds>(duration).count()) return;

		last_connect_time = high_resolution_clock::now();
		DisconnectClient(client_to_close);
		client_to_close++;
		return;
	}
	else if (DELAY_LIMIT < t_delay)
	{
		delay_multiplier = 10;
		return;
	}

	if (max_limit - (max_limit / 20) < active_clients) return;
	increasing = true;
	last_connect_time = high_resolution_clock::now();

	auto& last_client = everySessions[num_connections];

	last_client.client_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(everySessions[num_connections].client_socket, (sockaddr*)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (0 != Result)
	{
		int error = GetLastError();
		if (WSA_IO_PENDING != error)
		{
			error_display("WSAConnect : ", error);
		}
	}

	last_client.curr_packet_size = 0;
	last_client.prev_packet_data = 0;
	ZeroMemory(&last_client.recv_over, sizeof(last_client.recv_over));

	last_client.recv_over.event_type = OVERLAP_OPS::RECV;
	last_client.recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(last_client.recv_over.IOCP_buf);
	last_client.recv_over.wsabuf.len = sizeof(last_client.recv_over.IOCP_buf);

	DWORD recv_flag = 0;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(last_client.client_socket), g_hiocp, num_connections, 0);

	CSPacketSignIn l_packet{ "temp" };

	int temp = num_connections;
	sprintf_s(l_packet.Nickname, "%d", temp);

	SendPacket(num_connections, &l_packet);

	int ret = WSARecv(last_client.client_socket, &last_client.recv_over.wsabuf, 1,
		NULL, &recv_flag, &last_client.recv_over.over, NULL);
	if (SOCKET_ERROR == ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			error_display("RECV ERROR", err_no);
			goto fail_to_connect;
		}
	}

	num_connections++;

fail_to_connect:
	return;
}

void Test_Thread()
{
	while (true)
	{
		//Sleep(max(20, global_delay));
		Adjust_Number_Of_Client();

		for (int i = 0; i < num_connections; ++i)
		{
			if (!everySessions[i].connected)
			{
				continue;
			}

			if (everySessions[i].last_move_time + 1s > high_resolution_clock::now())
			{
				continue;
			}

			everySessions[i].last_move_time = high_resolution_clock::now();

			MOVE_TYPES dir{};
			switch (rand() % 4)
			{
				case 0: dir = MOVE_TYPES::LEFT; break;
				case 1: dir = MOVE_TYPES::RIGHT; break;
				case 2: dir = MOVE_TYPES::UP; break;
				case 3: dir = MOVE_TYPES::DOWN; break;
			}

			CSPacketMove my_packet{ CLIENTS_ORDER_BEGIN + PID(i), dir };

			my_packet.move_time = static_cast<unsigned>(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
			SendPacket(i, &my_packet);
		}
	}
}

void InitializeNetwork()
{
	for (auto& cl : everySessions)
	{
		cl.connected = false;
		cl.id = INVALID_ID;
	}

	for (auto& cl : client_map)
	{
		cl = -1;
	}

	num_connections = 0;
	last_connect_time = high_resolution_clock::now();

	WSADATA	wsadata{};
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	for (int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	test_thread = thread{ Test_Thread };
}

void ShutdownNetwork()
{
	test_thread.join();
	for (auto pth : worker_threads)
	{
		pth->join();
		delete pth;
	}
}

void Do_Network()
{
	return;
}

void GetPointCloud(int* size, float** points)
{
	int index = 0;
	for (int i = 0; i < num_connections; ++i)
		if (true == everySessions[i].connected)
		{
			point_cloud[index * 2] = static_cast<float>(everySessions[i].x);
			point_cloud[index * 2 + 1] = static_cast<float>(everySessions[i].y);
			index++;
		}

	*size = index;
	*points = point_cloud;
}

