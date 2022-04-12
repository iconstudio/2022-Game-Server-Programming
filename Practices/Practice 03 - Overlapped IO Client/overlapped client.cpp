#include "overlapped client.hpp"

using namespace std;
#define CASE01

void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ProceedSend(SOCKET sock);
void ProceedReceive(SOCKET sock);

const char* SERVER_ADDR = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUF_SIZE = 256;

SOCKET Socket;
char cbuffer_send[BUF_SIZE];
WSABUF buffer_send;
char cbuffer_receive[BUF_SIZE];
WSABUF buffer_receive;

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	Socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN svr_addr;
	int sz_addr = sizeof(svr_addr);
	ZeroMemory(&svr_addr, sz_addr);
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &svr_addr.sin_addr);

	WSAConnect(Socket, reinterpret_cast<sockaddr*>(&svr_addr), sz_addr, 0, 0, 0, 0);
	
	// 그냥 냅두면 수신이 아예 되지 않기 때문에 메인에서 한번은 무조건 실행해야 한다.
	ProceedReceive(Socket);
#ifdef CASE01
	ProceedSend(Socket);
#endif

	while (true)
	{
#ifdef CASE00
		// IO 병목 현상의 원인: cout -> 메인 스레드 강제 대기
		cout << "Enter Messsage: ";
		cin.getline(cbuffer_send, BUF_SIZE - 1);

		buffer_send.buf = cbuffer_send;
		buffer_send.len = static_cast<int>(strlen(cbuffer_send)) + 1;

		auto overlap = new WSAOVERLAPPED();
		ZeroMemory(overlap, sizeof(overlap));

		// RECV는 여기다 하면 안된다!!!!!
		// 왜냐하면 서버가 주도적으로 하지 않기 때문이다.
		WSASend(Socket, &buffer_send, 1, 0, 0, overlap, send_callback);

		// 이걸 안해주면 메인에서 콘솔의 점유율을 잡고 놓지 않기 때문에 recv 출력이 불가능하다.
		// 그런데 이것만 넣으면 서버가 늦게 보낼 수 밖에 없기 때문에 순서가 밀린다.
		// 비동기의 특징이다.
		// 그리고 cout 때문에 억지로 받는 것이기도 때문이다.
		// 실제 개발에선 메인 스레드는 무조건 프레임 당 실행을 보장 해야만 한다.
#endif
		SleepEx(100, TRUE);
	}

	closesocket(Socket);
	WSACleanup();

	return 0;
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes
	, LPWSAOVERLAPPED over, DWORD flags)
{
	SOCKET sock = (SOCKET)(over->hEvent);
	delete over;

	// 다시 시작
	ProceedSend(sock);
}

void ProceedSend(SOCKET sock)
{
	cout << "Enter Messsage: ";
	cin.getline(cbuffer_send, BUF_SIZE - 1);

	buffer_send.buf = cbuffer_send;
	buffer_send.len = static_cast<int>(strlen(cbuffer_send)) + 1;

	auto overlap = new WSAOVERLAPPED();
	ZeroMemory(overlap, sizeof(overlap));
	overlap->hEvent = (HANDLE)(sock);

	WSASend(sock, &buffer_send, 1, 0, 0, overlap, send_callback);
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes
	, LPWSAOVERLAPPED over, DWORD flags)
{
	cout << "Server Sent: " << cbuffer_receive << endl;

	delete over;
	ProceedReceive(Socket);
}

void ProceedReceive(SOCKET sock)
{
	auto overlap_recv = new WSAOVERLAPPED;
	ZeroMemory(overlap_recv, sizeof(*overlap_recv));

	DWORD flags = 0;
	buffer_receive.buf = cbuffer_receive;
	buffer_receive.len = BUF_SIZE;
	WSARecv(sock, &buffer_receive, 1, 0, &flags, overlap_recv, recv_callback);
}
