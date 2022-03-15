#pragma comment (lib, "WS2_32.LIB")
#include <iostream>
#include <WS2tcpip.h>
using namespace std;

const char* SERVER_ADDR = "127.0.0.1";
const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

void error_display(const char* msg, int err_no);

int main()
{
	wcout.imbue(locale("korean"));

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	int result = 0;
	for (;;)
	{
		char buf[BUFSIZE]{};
		ZeroMemory(buf, BUFSIZE);

		cout << "Enter Message : ";// cin.getline(buf, BUFSIZE);
		cin >> buf;

		DWORD sent_byte;
		WSABUF mybuf{};
		mybuf.buf = buf;
		mybuf.len = static_cast<ULONG>(strlen(buf)) + 1; // 문자열이라서 0까지 전송
		result = WSASend(s_socket, &mybuf, 1, &sent_byte, 0, 0, 0);
		if (SOCKET_ERROR == result)
		{
			error_display("WSARecv Error!\n", WSAGetLastError());
			break;
		}

		char recv_buf[BUFSIZE]{};
		WSABUF mybuf_r{};
		mybuf_r.buf = recv_buf;
		mybuf_r.len = BUFSIZE; // recv_buf의 모든 공간을 채워서 받기

		DWORD recv_byte;
		DWORD recv_flag = 0;
		result = WSARecv(s_socket, &mybuf_r, 1, &recv_byte, &recv_flag, 0, 0);
		if (SOCKET_ERROR == result)
		{
			error_display("WSARecv Error!\n", WSAGetLastError());
			break;
		}

		cout << "Server Sent [" << recv_byte << "bytes] : " << recv_buf << endl;
	}

	WSACleanup();	return 0;}void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcerr << L"에러 " << lpMsgBuf << std::endl;

	LocalFree(lpMsgBuf);
}
