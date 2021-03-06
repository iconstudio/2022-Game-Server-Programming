#include "ServerFramework.h"
#include "stdafx.h"
#include "ServerFramework.h"

ServerFramework::~ServerFramework()
{
	closesocket(Socket);
	WSACleanup();
}

void ServerFramework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == Socket)
	{
		ErrorDisplay("WSASocket()");
		return;
	}

	sz_Address = sizeof(Address);
	ZeroMemory(&Address, sz_Address);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = htonl(INADDR_ANY);
	Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(Socket, (SOCKADDR*)(&Address), sz_Address))
	{
		ErrorDisplay("bind()");
		return;
	}
}

void ServerFramework::Start()
{
	if (SOCKET_ERROR == listen(Socket, SOMAXCONN))
	{
		ErrorDisplay("listen()");
		return;
	}

	cout << "서버 시작\n";
	while (true)
	{
		if (!AcceptSession())
		{
			break;
		}
	}

	int result = 0;

	char recv_store[BUFFSIZE + 1];
	WSABUF buffer{};
	DWORD recv_size = 0;
	DWORD recv_flag = 0;
	DWORD send_size = 0;

	ZeroMemory(recv_store, BUFFSIZE + 1);
	buffer.buf = recv_store;
	buffer.len = BUFFSIZE;

	cout << "recv 1\n";
	result = WSARecv(client_socket, &buffer, 1, &recv_size, &recv_flag, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		ErrorDisplay("WSARecv 1");
		return;
	}

	auto player_pos = reinterpret_cast<Position*>(&recv_store);
	Player player{ player_pos->x, player_pos->y };
	cout << "플레이어 좌표: (" << player_pos->x << ", " << player_pos->y << ")\n";

	while (true)
	{
		ZeroMemory(recv_store, BUFFSIZE);
		buffer.buf = recv_store;
		buffer.len = BUFFSIZE;

		cout << "recv 2\n";
		result = WSARecv(client_socket, &buffer, 1, &recv_size, &recv_flag, NULL, NULL);
		if (SOCKET_ERROR == result)
		{
			ErrorDisplay("WSARecv 2");
			break;
		}

		if (0 < recv_size)
		{
			cout << "키 받음: " << recv_store << " (" << recv_size << " 바이트)\n";

			WPARAM received = 0;// = reinterpret_cast<WPARAM>(recv_store);
			memcpy_s(&received, sizeof(received), recv_store, recv_size);

			bool moved = false;
			switch (received)
			{
				case VK_LEFT:
				{
					moved = player.TryMoveLT();
				}
				break;

				case VK_RIGHT:
				{
					moved = player.TryMoveRT();
				}
				break;

				case VK_UP:
				{
					moved = player.TryMoveUP();
				}
				break;

				case VK_DOWN:
				{
					moved = player.TryMoveDW();
				}
				break;

				default:
				break;
			}

			cout << "send 1\n";
			buffer.buf = reinterpret_cast<char*>(&player);
			buffer.len = sizeof(player);
			result = WSASend(client_socket, &buffer, 1, &send_size, NULL, NULL, NULL);
			if (SOCKET_ERROR == result)
			{
				ErrorDisplay("WSASend 1");
				break;
			}

			if (!moved)
			{
				cout << "플레이어 움직이지 않음.\n";
			}
		} // if (0 < recv_size)
	} // while (true)
}

void CallbackStartPositions(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{

}

void CallbackInputs(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{

}

bool ServerFramework::AcceptSession()
{
	SOCKADDR_IN client_addr;
	int cl_addr_size = sizeof(client_addr);
	ZeroMemory(&client_addr, cl_addr_size);
	auto cl_addr_ptr = reinterpret_cast<SOCKADDR*>(&Address);

	SOCKET client_socket = WSAAccept(Socket, cl_addr_ptr, &sz_Address, NULL, NULL);
	if (INVALID_SOCKET == client_socket)
	{
		ErrorDisplay("WSAAccept()");
		return false;
	}

	cout << "클라이언트 접속: (" << inet_ntoa(client_addr.sin_addr)
		<< "), 핸들: " << client_socket << "\n";

	auto session = new Session;
	session->ID = Clients_index;
	session->Socket = client_socket;

	Clients.try_emplace(Clients_index, session);

	Clients_index++;
	return true;
}

Player* ServerFramework::CreatePlayerCharacter(Session* session)
{
	auto instance = new Player;
	session->Instance = instance;

	return (instance);
}

void ServerFramework::RemoveSession(const INT id)
{

}

bool Player::TryMoveLT()
{
	auto bd = BOARD_X;
	if (bd + CELL_SIZE < x)
	{
		x -= CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveRT()
{
	auto bd = BOARD_X + BOARD_W;
	if (x < bd - CELL_SIZE)
	{
		x += CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveUP()
{
	auto bd = BOARD_Y;
	if (bd + CELL_SIZE < y)
	{
		y -= CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveDW()
{
	auto bd = BOARD_Y + BOARD_H;
	if (y < bd - CELL_SIZE)
	{
		y += CELL_SIZE;
		return true;
	}
	return false;
}
