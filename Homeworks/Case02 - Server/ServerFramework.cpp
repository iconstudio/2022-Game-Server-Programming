#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework::ServerFramework()
	: Overlap()
	, Players_pool(), IndexedClients(), ClientsDict(), ClientsOverlap()
	, Clients_index_order(0), Clients_number(0), PlayerInst_index(0)
{
	ZeroMemory(&Overlap, sizeof(Overlap));

	InitializeCriticalSection(&Client_sect);

	IndexedClients.reserve(CLIENTS_MAX_NUMBER);
	ClientsDict.reserve(CLIENTS_MAX_NUMBER);
	ClientsOverlap.reserve(CLIENTS_MAX_NUMBER);
}

ServerFramework::~ServerFramework()
{
	DeleteCriticalSection(&Client_sect);
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
}

void ServerFramework::Start()
{
	if (SOCKET_ERROR == listen(Socket, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	cout << "서버 시작\n";
	while (true)
	{
		AcceptSession();
	}
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

	EnterCriticalSection(&Client_sect);
	auto session = new Session(this, client_socket);
	AddClient(Clients_index_order, session);
	AddClient(session->Overlap_recv, session);
	AddClient(session->Overlap_send_world, session);
	Clients_number++;

	session->ID = Clients_index_order++;

	session->ReceiveStartPosition();
	LeaveCriticalSection(&Client_sect);
}

void ServerFramework::BroadcastWorld()
{
	EnterCriticalSection(&Client_sect);
	if (0 < Clients_number)
	{
		for_each(ClientsDict.begin(), ClientsDict.end(), [](pair<const INT, Session*> set) {
			auto session = set.second;
			if (session && 0 == session->Size_send_world)
			{
				session->GenerateWorldData();
				session->SendWorld();
			}
		});
	}
	LeaveCriticalSection(&Client_sect);
}

UINT ServerFramework::GetClientsNumber() const
{
	return Clients_number;
}

void ServerFramework::AddClient(INT nid, Session* session)
{
	IndexedClients.push_back(nid);
	ClientsDict.emplace(nid, session);
}

void ServerFramework::AddClient(LPWSAOVERLAPPED overlap, Session* session)
{
	ClientsOverlap.emplace(overlap, session);
}

Session* ServerFramework::GetClientByIndex(INT nth)
{
	if (nth < Clients_number)
	{
		return GetClient(IndexedClients.at(nth));
	}

	return nullptr;
}

Session* ServerFramework::GetClient(INT fid)
{
	auto it = ClientsDict.find(fid);
	if (it != ClientsDict.cend())
	{
		return (*it).second;
	}

	return nullptr;
}

Session* ServerFramework::GetClient(LPWSAOVERLAPPED overlap)
{
	auto it = ClientsOverlap.find(overlap);
	if (it != ClientsOverlap.cend())
	{
		return (*it).second;
	}

	return nullptr;
}

void ServerFramework::RemoveClient(INT rid)
{
	auto it = find(IndexedClients.begin(), IndexedClients.end(), rid);
	if (it != IndexedClients.end())
	{
		IndexedClients.erase(it);
	}

	ClientsDict.erase(rid);
}

void ServerFramework::RemoveClient(LPWSAOVERLAPPED overlap)
{
	ClientsOverlap.erase(overlap);
}

void ServerFramework::RemoveSession(const INT id)
{
	EnterCriticalSection(&Client_sect);
	auto session = GetClient(id);

	session->Instance.reset();

	RemoveClient(id);
	RemoveClient(session->Overlap_recv);
	RemoveClient(session->Overlap_send_world);

	delete session;
	Clients_number--;
	LeaveCriticalSection(&Client_sect);
}

void ServerFramework::AssignPlayerInstance(shared_ptr<Player>& instance)
{
	instance = make_shared<Player>();
}

Player* ServerFramework::GetInstancesData(INT index)
{
	EnterCriticalSection(&Client_sect);
	auto player = GetClientByIndex(index);
	LeaveCriticalSection(&Client_sect);

	return (player->Instance).get();
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
