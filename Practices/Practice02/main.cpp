#pragma comment (lib, "WS2_32.LIB")
#include <WS2tcpip.h>
#include <iostream>
using namespace std;

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

void error_display(const char* msg, int err_no);

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(s_socket, SOMAXCONN);

	cout << "Server Started.\n";

	INT addr_size = sizeof(server_addr);
	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
	if (INVALID_SOCKET == c_socket)
	{
		error_display("WSAAccept Error!\n", WSAGetLastError());
		return 0;
	}

	SOCKADDR_IN client_addr;
	INT cl_addr_size = sizeof(client_addr);
	ZeroMemory(&client_addr, cl_addr_size);

	getpeername(c_socket, reinterpret_cast<sockaddr*>(&client_addr), &cl_addr_size);
	cout << "Client is connected: " << ntohl(client_addr.sin_addr.s_addr) << "\n";

	int result = 0;
	for (;;)
	{
		char recv_buf[BUFSIZE]{};
		WSABUF mybuf{};
		mybuf.buf = recv_buf; mybuf.len = BUFSIZE;
		cout << "Receving...\n";

		DWORD recv_byte = 0;
		DWORD recv_flag = 0;
		result = WSARecv(c_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
		if (SOCKET_ERROR == result)
		{
			error_display("WSARecv Error!\n", WSAGetLastError());
			break;
		}

		cout << "Client Sent [" << recv_byte << " bytes] : " << recv_buf << endl;

		DWORD sent_byte;
		mybuf.len = recv_byte;
		result = WSASend(c_socket, &mybuf, 1, &sent_byte, 0, 0, 0);
		if (SOCKET_ERROR == result)
		{
			error_display("WSASend Error!\n", WSAGetLastError());
			break;
		}
	}

	WSACleanup();

	return 0;
}

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
	std::wcerr << L"¿¡·¯ " << lpMsgBuf << std::endl;

	LocalFree(lpMsgBuf);
}

