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

	cout << "���� ����\n";
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
	cout << "Ŭ���̾�Ʈ ����: (" << inet_ntoa(client_addr.sin_addr)
		<< "), �ڵ�: " << client_socket << "\n";

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
	Position player{ player_pos->x, player_pos->y };
	cout << "�÷��̾� ��ǥ: (" << player_pos->x << ", " << player_pos->y << ")\n";

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
			cout << "Ű ����: " << recv_store << " (" << recv_size << " Bytes)\n";

			auto received = reinterpret_cast<WPARAM>(&recv_store);

			switch (received)
			{
				case VK_LEFT:
				{
					player.x -= 32;
				}
				break;

				case VK_RIGHT:
				{
					player.x += 32;
				}
				break;

				case VK_UP:
				{
					player.y -= 32;
				}
				break;

				case VK_DOWN:
				{
					player.y += 32;
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

			auto position = reinterpret_cast<Position*>(buffer.buf);

			cout << "��ġ ����: (" << position->x << ", " << position->y << ")\n";
		} // if (0 < recv_size)
	} // while (true)

	closesocket(m_Socket);
	WSACleanup();

	return 0;
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

	wcout << title << " -> ����: " << lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}
