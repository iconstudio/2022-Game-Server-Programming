#include "Server.h"

using namespace std;

SOCKET m_Socket;
SOCKADDR_IN m_Address;
INT sz_Address;

void ErrorDisplay(const char* title);

int main()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == m_Socket)
	{
		ErrorDisplay("WSASocket()");
		return;
	}

	sz_Address = sizeof(m_Address);
	ZeroMemory(&m_Address, sz_Address);
	m_Address.sin_family = AF_INET;
	m_Address.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(m_Socket, (SOCKADDR*)(&m_Address), sz_Address))
	{
		ErrorDisplay("bind()");
		return;
	}

	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		ErrorDisplay("listen()");
		return;
	}

	cout << "Server opened\n";
	SOCKET Client = accept(m_Socket, (SOCKADDR*)(&m_Address), &sz_Address);

	int result = 0;
	WSABUF buffer{};
	DWORD sz_recv = 0;
	while (true)
	{
		result = WSARecv(Client, &buffer, 1, &sz_recv, 0, NULL, NULL);
		if (SOCKET_ERROR == result)
		{
			ErrorDisplay("WSARecv");
			break;
		}
	}

	return 0;
}

void ErrorDisplay(const char* title)
{
	CHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	cerr << title << "¿¡·¯: " << lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}
