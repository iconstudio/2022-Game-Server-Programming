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
	
	// �׳� ���θ� ������ �ƿ� ���� �ʱ� ������ ���ο��� �ѹ��� ������ �����ؾ� �Ѵ�.
	ProceedReceive(Socket);
#ifdef CASE01
	ProceedSend(Socket);
#endif

	while (true)
	{
#ifdef CASE00
		// IO ���� ������ ����: cout -> ���� ������ ���� ���
		cout << "Enter Messsage: ";
		cin.getline(cbuffer_send, BUF_SIZE - 1);

		buffer_send.buf = cbuffer_send;
		buffer_send.len = static_cast<int>(strlen(cbuffer_send)) + 1;

		auto overlap = new WSAOVERLAPPED();
		ZeroMemory(overlap, sizeof(overlap));

		// RECV�� ����� �ϸ� �ȵȴ�!!!!!
		// �ֳ��ϸ� ������ �ֵ������� ���� �ʱ� �����̴�.
		WSASend(Socket, &buffer_send, 1, 0, 0, overlap, send_callback);

		// �̰� �����ָ� ���ο��� �ܼ��� �������� ��� ���� �ʱ� ������ recv ����� �Ұ����ϴ�.
		// �׷��� �̰͸� ������ ������ �ʰ� ���� �� �ۿ� ���� ������ ������ �и���.
		// �񵿱��� Ư¡�̴�.
		// �׸��� cout ������ ������ �޴� ���̱⵵ �����̴�.
		// ���� ���߿��� ���� ������� ������ ������ �� ������ ���� �ؾ߸� �Ѵ�.
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

	// �ٽ� ����
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
