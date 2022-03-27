#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework::ServerFramework()
	: Overlap(), World(), Instances_blob()
	, Clients(), OverlapClients(), Clients_index(0), Clients_number(0)
{
	ZeroMemory(&Overlap, sizeof(Overlap));

	Clients.reserve(CLIENTS_MAX_NUMBER);
	OverlapClients.reserve(CLIENTS_MAX_NUMBER);
	World.reserve(CLIENTS_MAX_NUMBER);
}

ServerFramework::~ServerFramework()
{
	closesocket(Socket);
	WSACleanup();
}

void ServerFramework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == Socket)
	{
		ErrorDisplay("WSASocket()");
		return;
	}

	sz_Address = sizeof(Address);
	ZeroMemory(&Address, sz_Address);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = htonl(INADDR_ANY);
	Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(Socket, (SOCKADDR*)(&Address), sz_Address))
	{
		ErrorDisplay("bind()");
		return;
	}

	Event_world = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (0 == Event_world)
	{
		ErrorDisplay("CreateEvent()");
		return;
	}
}

void ServerFramework::Start()
{
	if (SOCKET_ERROR == listen(Socket, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	auto worker = CreateThread(NULL, 0, Communicate, this, 0, NULL);
	if (0 == worker)
	{
		ErrorDisplay("CreateThread()");
		return;
	}

	while (true)
	{
		WaitForSingleObjectEx(Event_world, 200, TRUE);
		GenerateInstancesData();
		//SleepEx(200, TRUE);
	}
}

UINT ServerFramework::GetClientsNumber() const
{
	return Clients_number;//Clients.size();
}

void ServerFramework::AddClient(INT nid, Session* session)
{
	Clients_number++;
	Clients.emplace(nid, session);
}

void ServerFramework::AddClient(LPWSAOVERLAPPED overlap, Session* session)
{
	OverlapClients.emplace(overlap, session);
}

Session* ServerFramework::GetClient(INT fid)
{
	auto it = Clients.find(fid);
	if (it != Clients.cend())
	{
		return (*it).second;
	}

	return nullptr;
}

Session* ServerFramework::GetClient(LPWSAOVERLAPPED overlap)
{
	auto it = OverlapClients.find(overlap);
	if (it != OverlapClients.cend())
	{
		return (*it).second;
	}

	return nullptr;
}

void ServerFramework::RemoveClient(INT nid)
{
	Clients_number--;
	Clients.erase(nid);
}

void ServerFramework::RemoveClient(LPWSAOVERLAPPED overlap)
{
	OverlapClients.erase(overlap);
}

void ServerFramework::RemoveInstance(Position* instance)
{
	auto fid = find(World.cbegin(), World.cend(), instance);
	if (World.cend() != fid)
	{
		World.erase(fid);
	}
}

void ServerFramework::RemoveSession(const INT id)
{
	auto session = GetClient(id);
	RemoveClient(id);
	delete session;
}

void ServerFramework::AcceptSession()
{
	SOCKADDR_IN client_addr;
	int cl_addr_size = sizeof(client_addr);
	ZeroMemory(&client_addr, cl_addr_size);
	auto cl_addr_ptr = reinterpret_cast<SOCKADDR*>(&Address);

	SOCKET client_socket = WSAAccept(Socket, cl_addr_ptr, &sz_Address, NULL, NULL);
	if (INVALID_SOCKET == client_socket)
	{
		ErrorDisplay("WSAAccept()");
		return;
	}

	cout << "클라이언트 접속: (" << inet_ntoa(client_addr.sin_addr)
		<< "), 핸들: " << client_socket << "\n";

	auto session = new Session(this, client_socket);
	AddClient(Clients_index, session);

	session->ID = Clients_index;
	session->ReceiveStartPosition();

	Clients_index++;
	CastWorldChanged();
}

void ServerFramework::AddInstance(Position* instance)
{
	World.emplace_back(instance);
}

Position* ServerFramework::GenerateInstancesData()
{
	auto result = new Position[Clients_number]{};
	if (0 < Clients_number)
	{
		int i = 0;
		for_each(World.begin(), World.end(), [&](Position* instance) {
			(result[i++]) = (*instance);
		});
	}

	return result;
}

Position* ServerFramework::GetInstancesData()
{
	return Instances_blob;
}

void ServerFramework::CastWorldChanged()
{
	SetEvent(Event_world);
}

bool Player::TryMoveLT()
{
	auto bd = BOARD_X;
	if (bd + CELL_SIZE < x)
	{
		x -= CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveRT()
{
	auto bd = BOARD_X + BOARD_W;
	if (x < bd - CELL_SIZE)
	{
		x += CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveUP()
{
	auto bd = BOARD_Y;
	if (bd + CELL_SIZE < y)
	{
		y -= CELL_SIZE;
		return true;
	}
	return false;
}

bool Player::TryMoveDW()
{
	auto bd = BOARD_Y + BOARD_H;
	if (y < bd - CELL_SIZE)
	{
		y += CELL_SIZE;
		return true;
	}
	return false;
}
