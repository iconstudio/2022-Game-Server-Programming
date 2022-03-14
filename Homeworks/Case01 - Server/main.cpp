#include "main.h"

SOCKET m_Socket;
SOCKADDR_IN m_Address;
INT sz_Address;

int main()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		//ErrorAbort("WSAStartup()");
		return;
	}

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		//ErrorAbort("socket()");
		return;
	}

	sz_Address = sizeof(m_Address);
	ZeroMemory(&m_Address, sz_Address);
	m_Address.sin_family = AF_INET;
	m_Address.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(m_Socket, (SOCKADDR*)(&m_Address), sz_Address))
	{
		//ErrorAbort("bind()");
		return;
	}

	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		//ErrorAbort("listen()");
		return;
	}

	SOCKET Client = accept(m_Socket, (SOCKADDR*)(&m_Address), &sz_Address);

	int result = 0;
	WSABUF buffer{};
	DWORD sz_recv = 0;
	while (true)
	{
		result = WSARecv(Client, &buffer, 1, &sz_recv, 0, NULL, NULL);
	}

	return 0;
}