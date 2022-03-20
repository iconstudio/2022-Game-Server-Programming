#include "Server.h"

SOCKET m_Socket;
SOCKADDR_IN m_Address;
INT sz_Address;

int main()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return 0;
	}

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == m_Socket)
	{
		ErrorDisplay("WSASocket()");
		return 0;
	}

	sz_Address = sizeof(m_Address);
	ZeroMemory(&m_Address, sz_Address);
	m_Address.sin_family = AF_INET;
	m_Address.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(m_Socket, (SOCKADDR*)(&m_Address), sz_Address))
	{
		ErrorDisplay("bind()");
		return 0;
	}

	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		ErrorDisplay("listen()");
		return 0;
	}

	cout << "서버 시작\n";
	SOCKET client_socket = WSAAccept(m_Socket, (SOCKADDR*)(&m_Address), &sz_Address, NULL, NULL);
	if (INVALID_SOCKET == client_socket)
	{
		ErrorDisplay("WSAAccept()");
		return 0;
	}

	SOCKADDR_IN client_addr;
	int cl_addr_size = sizeof(client_addr);
	ZeroMemory(&client_addr, cl_addr_size);
	if (SOCKET_ERROR == getpeername(client_socket, reinterpret_cast<SOCKADDR*>(&client_addr), &cl_addr_size))
	{
		ErrorDisplay("getpeername()");
		return 0;
	}
	cout << "클라이언트 접속: (" << inet_ntoa(client_addr.sin_addr)
		<< "), 핸들: " << client_socket << "\n";

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
		return 0;
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

			if (moved)
			{
				cout << "send 1\n";
				buffer.buf = reinterpret_cast<char*>(&player);
				buffer.len = sizeof(player);
				result = WSASend(client_socket, &buffer, 1, &send_size, NULL, NULL, NULL);
				if (SOCKET_ERROR == result)
				{
					ErrorDisplay("WSASend 1");
					break;
				}
			}
			else
			{
				cout << "플레이어 움직이지 않음.\n";
			}
		} // if (0 < recv_size)
	} // while (true)

	closesocket(m_Socket);
	WSACleanup();

	return 0;
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

void ErrorDisplay(const char* title)
{
	TCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	wcout << title << " -> 오류: " << lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}
