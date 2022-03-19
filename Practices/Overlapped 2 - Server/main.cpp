#include "main.h"

HANDLE Event_write, Event_read;
HANDLE Server_worker;
SOCKET Server_socket;
SOCKET Client_socket;

int main()
{
	int result = 0;

	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		throw std::runtime_error("Startup Error");
	}

	Server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == Server_socket)
	{
		throw std::runtime_error("Socket Creation Error");
	}


	SOCKADDR_IN address;
	int sz_address = sizeof(address);
	ZeroMemory(&address, sz_address);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(SERVER_PORT);

	auto ptr_address = reinterpret_cast<SOCKADDR*>(&address);
	if (SOCKET_ERROR == bind(Server_socket, ptr_address, sz_address))
	{
		throw std::runtime_error("Socket Binding Error");
	}

	if (SOCKET_ERROR == listen(Server_socket, SOMAXCONN))
	{
		throw std::runtime_error("Socket Listening Error");
	}

	Event_write = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (NULL == Event_write) { return 1; }
	Event_read = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == Event_read) { return 1; }

	std::cout << "���� ����\n";
	Server_worker = CreateThread(NULL, 0, ServerIOWorker, NULL, 0, NULL);
	if (NULL == Server_worker)
	{
		throw std::runtime_error("Creating Thread Error");
		return 1;
	}
	CloseHandle(Server_worker);

	std::cout << "���� �޴� ��...\n";
	while (true)
	{
		WaitForSingleObject(Event_write, INFINITE);

		Client_socket = WSAAccept(Server_socket, NULL, 0, NULL, NULL);
		if (INVALID_SOCKET == Client_socket)
		{
			throw std::runtime_error("Socket Connection Error");
			break;
		}

		SetEvent(Event_read);
	}

	WSACleanup();

	return 0;
}

DWORD __stdcall ServerIOWorker(LPVOID param)
{
	int result = 0;

	while (true)
	{
		while (true)
		{
			// Alertable wait
			DWORD awaited = WaitForSingleObjectEx(Event_read, INFINITE, TRUE);
			if (WAIT_OBJECT_0 == awaited) break;
			else if (WAIT_IO_COMPLETION != awaited) return 1;
		}

		std::cout << "Ŭ���̾�Ʈ ����\n";

		auto client = new Client;
		if (!client)
		{
			std::cout << "�޸� ����!\n";
			throw std::bad_exception();
		}

		auto& overlap = client->Overlap;
		auto& sk = client->Socket;
		auto& buff = client->Buffer;
		SetEvent(Event_write);

		sk = Client_socket;
		buff.buf = client->Back_buffer;
		buff.len = ::SZ_BUFFER;

		std::cout << "�񵿱� ����� ���� (����)\n";
		DWORD sz_recv = 0;
		DWORD flags = 0;

		result = WSARecv(client->Socket, &buff, 1, &sz_recv, &flags
			, &overlap, CompletionRoutine);
		if (SOCKET_ERROR == result)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				throw std::runtime_error("WSARecv: ����� ���� ����");
				return 1;
			}
		}
	}

	return 0;
}

void CompletionRoutine(DWORD error, DWORD traffic, LPOVERLAPPED overlapped, DWORD flags)
{
	int result = 0;

	auto client = reinterpret_cast<Client*>(overlapped);
	auto cli_socket = client->Socket;
	auto& cli_overlap = client->Overlap;
	auto& cli_buffer = client->Buffer;
	auto& cli_cbuffer = client->Back_buffer;
	auto& cli_sz_recv = client->sz_recv;
	auto& cli_sz_sent = client->sz_sent;

	if (error != 0 || traffic == 0)
	{
		closesocket(client->Socket);
		std::cout << "Ŭ���̾�Ʈ ����: " << client->Socket << ".\n";
		delete client;
		return;
	}

	if (0 == cli_sz_recv)
	{
		cli_sz_recv = traffic;
		cli_sz_sent = 0;

		std::cout << "����: " << cli_buffer.buf << " (" << cli_sz_recv << " ����Ʈ)\n";
	}
	else
	{
		cli_sz_sent += traffic;
	}

	if (cli_sz_sent < cli_sz_recv) // ������
	{
		std::cout << "�۽� ����\n";

		ZeroMemory(&cli_overlap, sizeof(cli_overlap));
		cli_buffer.buf = cli_cbuffer + cli_sz_sent;
		cli_buffer.len = cli_sz_recv - cli_sz_sent;

		DWORD sz_sent = 0;
		result = WSASend(cli_socket, &cli_buffer, 1, &sz_sent, 0
			, &cli_overlap, CompletionRoutine);
		if (SOCKET_ERROR == result)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				throw std::runtime_error("WSASend: ����� ���� ����");
				return;
			}
		}
	}
	else // �ޱ�
	{
		std::cout << "���� ����\n";
		cli_sz_recv = 0;

		ZeroMemory(&cli_overlap, sizeof(cli_overlap));
		cli_buffer.buf = cli_cbuffer;
		cli_buffer.len = ::SZ_BUFFER;

		DWORD sz_recv = 0;
		DWORD flags = 0;

		result = WSARecv(cli_socket, &cli_buffer, 1, &sz_recv, &flags
			, &cli_overlap, CompletionRoutine);
		if (SOCKET_ERROR == result)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				throw std::runtime_error("WSARecv: ����� ���� ����");
				return;
			}
		}
	}
}

Client::Client()
	: Back_buffer{}, sz_recv(0), sz_sent(0)
{
	ZeroMemory(&Overlap, sizeof(Overlap));
}
