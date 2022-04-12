#include "iocp server.hpp"

const DWORD serverKey = 99999;

HANDLE completionPort = NULL;
SOCKET listener;
SOCKET client_sock;

OVER_EXP acceptOverlap{};
WSABUF acceptBuffer{};
CHAR acceptCBuffer[BUF_SIZE]{};

array<SESSION, MAX_USER> ClientIDs;
unordered_map<UINT, SESSION*> Clients;
unordered_map<WSAOVERLAPPED*, int> over_to_session;

std::vector<std::thread> Workers{ THREADS_CNT };

UINT orderClients = 0;
UINT numberClients = 0;

void do_accept();
void Worker(const UINT index);
int GetNewbieID();
void NotifyPlayerConnectionToAll(const UINT pid);
void NotifyPlayerDisconnectionToAll(const UINT pid);
void NotifyPlayerActionToAll(const UINT pid);
void Disconnect(const UINT pid);

void ProcessPacket(void* data, UINT sz_remain)
{
	auto packet = reinterpret_cast<PACKET*>(data);
	auto size = packet->Size;
	auto type = packet->Type;
	auto pid = packet->ID;

	auto session = Clients.at(pid);

	switch (type)
	{
		case CS_LOGIN:
		{
			auto result = reinterpret_cast<CSPacketLogin*>(data);

			auto nid = result->ID;
			session->_id = nid;

			strcpy_s(session->Name, result->Name);

			//
		}
		break;

		case CS_MOVE:
		{
			auto result = reinterpret_cast<CSPacketMove*>(data);

			session->x += result->aX;
			session->y += result->aY;

			if (session->x < 0) session->x = 0;
			else if (WORLD_SZ <= session->x) session->x = WORLD_SZ - 1;

			if (session->y < 0) session->y = 0;
			else if (WORLD_SZ <= session->y) session->y = WORLD_SZ - 1;
		}
		break;

		case SC_ACCEPT:
		{

		}
		break;

		case SC_ADD_PLAYER:
		{

		}
		break;

		case SC_RMV_PLAYER:
		{

		}
		break;

		case SC_MOV_PLAYER:
		{

		}
		break;
	}

}

int main()
{
	int checksum = 0;

	WSADATA WSAData;
	checksum = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (SOCKET_ERROR == checksum)
	{
		cout << "WSAStartup() 오류!\n";
		return;
	}

	SOCKET listener = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listener)
	{
		cout << "WSASocket(listener) 오류!\n";
		return;
	}

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	auto server_addr_ptr = reinterpret_cast<SOCKADDR*>(&server_addr);
	checksum = bind(listener, server_addr_ptr, sizeof(server_addr));
	if (SOCKET_ERROR == checksum)
	{
		cout << "bind() 오류! \n";
		return;
	}

	checksum = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == checksum)
	{
		cout << "listen() 오류! \n";
		return;
	}

	acceptOverlap.Completion_type = ACCEPT;
	acceptOverlap.recvBuffer = acceptBuffer;

	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort)
	{
		cout << "CreateIoCompletionPort(INVALID_HANDLE_VALUE) 오류! \n";
		return;
	}

	CreateIoCompletionPort(HANDLE(listener), completionPort, serverKey, 0);

	cout << "서버 시작\n";
	acceptBuffer.buf = acceptCBuffer;
	acceptBuffer.len = BUF_SIZE;
	do_accept();
	
	Workers.resize(THREADS_CNT);
	for (int i = 0; i < THREADS_CNT; ++i)
	{
		Workers.emplace_back(Worker, 0);
	}

	while (true);

	closesocket(listener);
	WSACleanup();
}

void do_accept()
{
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	SOCKET c_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	const auto sz_addr = sizeof(SOCKADDR_IN) + 16;
	auto try_accept = AcceptEx(listener, c_sock, NULL
		, 0, sz_addr, sz_addr, NULL
		, &acceptOverlap);
}

void Worker(const UINT index)
{
	LPWSAOVERLAPPED overlap = NULL;
	DWORD bytes = 0;
	ULONGLONG key = 0;

	while (true)
	{
		auto gqcs = GetQueuedCompletionStatus(completionPort, &bytes, &key, &overlap, INFINITE);

		if (TRUE == gqcs)
		{
			auto over = reinterpret_cast<OVER_EXP*>(&overlap);
			auto type = over->Completion_type;

			switch (type)
			{
				case ACCEPT:
				{
					if (MAX_USER <= numberClients)
					{
						cout << "Cannot accept the client!\n";
						closesocket(client_sock);
					}
					else
					{
						int new_id = GetNewbieID();

						auto session = new SESSION(orderClients, client_sock);
						CreateIoCompletionPort(HANDLE(client_sock), completionPort, new_id, 0);

						session->used = true;
						Clients.try_emplace(orderClients, session);

						session->do_recv();

						cout << "Client (ID: " << orderClients << ") connected.\n";
						orderClients++;
						numberClients++;
					}

					ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
					do_accept();
				}
				break;

				case RECV:
				{
					auto session = Clients[key];

					if (session)
					{
						auto& recv_remain = session->recv_remain;
						auto& recv_wbuffer = session->recvOverlap.recvBuffer;
						auto& recv_cbuffer = recv_wbuffer.buf;
						auto& recv_size = recv_wbuffer.len;

						recv_remain += bytes;

						const auto sz_want = sizeof(PACKET);

						while (0 < recv_remain)
						{
							auto packet_size = int(&recv_cbuffer[0]);

							ProcessPacket(recv_cbuffer, recv_remain);

							recv_cbuffer += recv_remain;
							recv_remain -= packet_size;
						}

						//session->recv_remain = re

					}
				}
				break;

				case SEND:
				{

				}
				break;
			}
		}
	}
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	auto sdata = reinterpret_cast<OVER_EXP*>(over);

	delete[] sdata->sendCBuffer;
	delete sdata;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int  client_id = over_to_session[over];
	if (0 == num_bytes)
	{
		cout << "Client disconnected\n";
		Clients.erase(client_id);
		over_to_session.erase(over);
		return;
	};

	cout << "Client " << client_id << " sent: " << Clients[client_id]->_c_mess << endl;

	for (auto& cl : Clients)
	{
		auto session = cl.second;
		session->do_send(session->_c_mess);
	}

	Clients[client_id]->do_recv();
}

int GetNewbieID()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		auto session = Clients[i];
		if (!session->used)
		{
			return i;
		}
	}

	return -1;
}

void Disconnect(const UINT pid)
{
	auto session = Clients[pid];
	if (session->used)
	{
		session->used = false;
		//delete session;

		NotifyPlayerDisconnectionToAll(pid);
	}
}

void NotifyPlayerConnectionToAll(const UINT pid)
{
	for (auto& player : Clients)
	{
		auto target_session = player.second;
		if (target_session->used && target_session->_id != pid)
		{
			auto packet = new SCPacketAccept(pid);

			target_session->do_send(packet);
		}
	}
}

void NotifyPlayerDisconnectionToAll(const UINT pid)
{
	for (auto& player : Clients)
	{
		auto target_session = player.second;
		if (target_session->used && target_session->_id != pid)
		{
			auto packet = new SCPacketRemovePlayer(pid);

			target_session->do_send(packet);
		}
	}
}

void NotifyPlayerActionToAll(const UINT pid)
{

}
