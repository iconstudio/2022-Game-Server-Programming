#include "overlapped server.hpp"

using namespace std;

const char* SERVER_ADDR = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUF_SIZE = 256;

SOCKET Socket;
char cbuffer_send[BUF_SIZE];
WSABUF buffer_send;
char cbuffer_receive[BUF_SIZE];
WSABUF buffer_receive;
WSAOVERLAPPED Overlap;

void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);
void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ProceedSend(SOCKET sock);
void ProceedReceive(SOCKET sock);

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s_socket)
	{
		ErrorDisplay(L"WSASocket Error!\n");
		return 0;
	}

	SOCKADDR_IN server_addr;
	INT server_addr_sz = sizeof(server_addr);
	ZeroMemory(&server_addr, server_addr_sz);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), server_addr_sz);
	listen(s_socket, SOMAXCONN);

	cout << "Server Started.\n";

	SOCKADDR_IN client_addr;
	INT client_addr_sz = sizeof(client_addr);
	ZeroMemory(&client_addr, client_addr_sz);

	auto client_addr_ptr = reinterpret_cast<SOCKADDR*>(&client_addr);
	SOCKET c_socket = WSAAccept(s_socket
		, client_addr_ptr, &client_addr_sz, 0, 0);
	if (INVALID_SOCKET == c_socket)
	{
		ErrorDisplay(L"WSAAccept Error!\n");
		return 0;
	}

	getpeername(c_socket, client_addr_ptr, &client_addr_sz);
	cout << "Client is connected: " << ntohl(client_addr.sin_addr.s_addr) << "\n";

	ProceedReceive(c_socket);

	for (;;)
	{
		SleepEx(300, TRUE);
	}

	closesocket(s_socket);
	WSACleanup();

	return 0;
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes
	, LPWSAOVERLAPPED over, DWORD flags)
{
	SOCKET sock = (SOCKET)(over->hEvent);

	ProceedReceive(sock);
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes
	, LPWSAOVERLAPPED over, DWORD flags)
{
	cout << "Client Sent [" << num_bytes << " bytes]: " << cbuffer_receive << endl;

	buffer_send.buf = cbuffer_receive;
	buffer_send.len = static_cast<int>(strlen(cbuffer_receive)) + 1;

	SOCKET sock = (SOCKET)(over->hEvent);
	ZeroMemory(over, sizeof(*over));
	over->hEvent = (HANDLE)(sock);

	int result = WSASend(sock, &buffer_send, 1, 0, 0, over, send_callback);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		ErrorDisplay(L"WSASend Error!\n");
		return;
	}
}

void ProceedReceive(SOCKET sock)
{
	ZeroMemory(&Overlap, sizeof(Overlap));
	Overlap.hEvent = (HANDLE)(sock);

	buffer_receive.buf = cbuffer_receive;
	buffer_receive.len = BUF_SIZE;
	cout << "Receving...\n";

	DWORD recv_flag = 0;
	int result = WSARecv(sock, &buffer_receive, 1, 0, &recv_flag
		, &Overlap, recv_callback);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		ErrorDisplay(L"WSARecv Error!\n");
		return;
	}
}

void ErrorAbort(const wchar_t* title)
{
	WCHAR* lpMsgBuf;
	auto error = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	WCHAR wtitle[512];
	wsprintf(wtitle, L"오류: %s", title);

	MessageBox(NULL, lpMsgBuf, wtitle, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	exit(error);
}

void ErrorDisplay(const wchar_t* title)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	WCHAR wtitle[512];
	wsprintf(wtitle, L"오류: %s", title);

	MessageBox(NULL, lpMsgBuf, wtitle, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
}
