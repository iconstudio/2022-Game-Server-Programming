#include "mplay overlapped server.hpp"

using namespace std;

const char* SERVER_ADDR = "127.0.0.1";
constexpr short SERVER_PORT = 4000;

SOCKET Server_socket;
INT client_number = 0;
unordered_map<LPWSAOVERLAPPED, SOCKET> Socket_dict;
unordered_map<SOCKET, Session*> Client_dict;

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	Server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == Server_socket)
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

	bind(Server_socket, reinterpret_cast<SOCKADDR*>(&server_addr), server_addr_sz);
	listen(Server_socket, SOMAXCONN);

	cout << "Server Started.\n";

	SOCKADDR_IN client_addr;
	INT client_addr_sz = sizeof(client_addr);
	ZeroMemory(&client_addr, client_addr_sz);
	auto client_addr_ptr = reinterpret_cast<SOCKADDR*>(&client_addr);

	while (true)
	{
		SOCKET c_socket = WSAAccept(Server_socket
			, client_addr_ptr, &client_addr_sz, 0, 0);
		if (INVALID_SOCKET == c_socket)
		{
			ErrorDisplay(L"WSAAccept Error!\n");
			return 0;
		}

		getpeername(c_socket, client_addr_ptr, &client_addr_sz);
		cout << "Client is connected: " << ntohl(client_addr.sin_addr.s_addr) << "\n";

		auto client = new Session(client_number++, c_socket);
		client->ProceedReceive();
		Client_dict.try_emplace(c_socket, move(client));
	}
}

void RecvRoutine(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{
	auto client_socket = Socket_dict[overlap];
	auto client = Client_dict[client_socket];
	if (0 != err || 0 == num_bytes)
	{
		Socket_dict.erase(overlap);
		Client_dict.erase(client_socket);

		return;
	}

	auto& buf_recv = client->Buffer_recv;
	auto& buf_send = client->Buffer_send;
	auto& cbuf_recv = client->cbuffer_recv;

	cout << "Client Sent [" << num_bytes << " bytes]: " << cbuf_recv << endl;

	buf_send.len = static_cast<int>(strlen(cbuf_recv)) + 1;

	client->ClearOverlap();

	int result = WSASend(client_socket, &buf_recv, 1, 0, 0, overlap, SendRoutine);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		ErrorDisplay(L"WSASend Error!\n");
		return;
	}
}

void SendRoutine(DWORD, DWORD, LPWSAOVERLAPPED overlap, DWORD)
{
	auto client_socket = Socket_dict[overlap];
	auto client = Client_dict[client_socket];
	client->ProceedReceive();
}

Session::Session() : Overlap(), Buffer_recv(), Buffer_send()
{
	ClearOverlap();

	Buffer_recv.buf = cbuffer_recv;
	Buffer_recv.len = BUF_SIZE;
	Buffer_send.buf = cbuffer_recv;
	Buffer_send.len = 0;
}

Session::Session(INT nid, SOCKET sock)
	: id(nid), Overlap(), Buffer_recv(), Buffer_send()
{
	Socket_dict[&Overlap] = sock;

	Socket = sock;

	ClearOverlap();

	Buffer_recv.buf = cbuffer_recv;
	Buffer_recv.len = BUF_SIZE;
	Buffer_send.buf = cbuffer_recv;
	Buffer_send.len = 0;
}

void Session::ClearOverlap()
{
	ZeroMemory(&Overlap, sizeof(Overlap));
}

void Session::ProceedReceive()
{
	DWORD recv_flag = 0;
	ClearOverlap();
	int result = WSARecv(Socket, &Buffer_recv, 1, NULL, &recv_flag
		, &Overlap, RecvRoutine);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		ErrorDisplay(L"ProceedReceive Error!\n");
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
