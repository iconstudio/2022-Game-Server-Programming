#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

class SESSION;
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

unordered_map<int, SESSION> clients;
unordered_map<WSAOVERLAPPED*, int> over_to_session;

class SEND_DATA {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	SEND_DATA(char* n_data, int size, int client_id)
	{
		_wsabuf.len = size + 2;
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_send_buf[0] = size + 2;
		_send_buf[1] = client_id;
		memcpy(_send_buf + 2, n_data, size);
	}
};

class SESSION {
	WSAOVERLAPPED _c_over;
	WSABUF _c_wsabuf[1];
public:
	int _id;
	SOCKET _socket;
	CHAR _c_mess[BUF_SIZE];
public:
	SESSION() {}
	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_c_wsabuf[0].buf = _c_mess;
		_c_wsabuf[0].len = sizeof(_c_mess);
		over_to_session[&_c_over] = id;
	}
	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_c_over, 0, sizeof(_c_over));
		WSARecv(_socket, _c_wsabuf, 1, 0, &recv_flag, &_c_over, recv_callback);
	}

	void do_send(int num_bytes, char *mess, int client_id)
	{
		SEND_DATA* sdata = new SEND_DATA{ mess, num_bytes, client_id };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, send_callback);
	}
};

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	SEND_DATA* sdata = reinterpret_cast<SEND_DATA *>(over);
	delete sdata;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int  client_id = over_to_session[over];
	if (0 == num_bytes) {
		cout << "Client disconnected\n";
		clients.erase(client_id);
		over_to_session.erase(over);
		return;
	};
	cout << "Client " << client_id << " sent: " << clients[client_id]._c_mess << endl;
	for (auto& cl : clients)
		cl.second.do_send(num_bytes, clients[client_id]._c_mess, client_id);
	clients[client_id].do_recv();
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;
	while (true) {
		SOCKET client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, 0, 0);
		clients.try_emplace(client_id, client_id, client);
		clients[client_id].do_recv();
		cout << "Client " << client_id << " connected.\n";
		client_id++;
	}
	closesocket(server);
	WSACleanup();
}
