#include "iocp server.hpp"

const DWORD serverKey = 99999;

SOCKET listener;
SOCKET client_sock;
OVER_EXP acceptOverlap{};

array<SESSION, MAX_USER> ClientIDs;
unordered_map<UINT, SESSION*> Clients;
unordered_map<WSAOVERLAPPED*, int> over_to_session;

UINT orderClients = 0;
UINT numberClients = 0;

void do_accept();

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

			session->_id = result->ID;

			strcpy_s(session->Name, result->Name);
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
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET listener = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listener, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	listen(listener, SOMAXCONN);

	HANDLE completionPort;

	WSABUF acceptBuffer{};
	CHAR acceptCBuffer[BUF_SIZE]{};
	acceptBuffer.buf = acceptCBuffer;
	acceptBuffer.len = BUF_SIZE;

	acceptOverlap.Completion_type = ACCEPT;
	acceptOverlap.recvBuffer = acceptBuffer;

	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(HANDLE(listener), completionPort, serverKey, 0);

	cout << "서버 시작\n";
	do_accept();

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
			}
		}
	}

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
		session->do_send(client_id, session->_c_mess);
	}

	Clients[client_id]->do_recv();
}

