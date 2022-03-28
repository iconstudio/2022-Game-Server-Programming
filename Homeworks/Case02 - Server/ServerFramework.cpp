#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework::ServerFramework()
	: Overlap(), PlayerInst_pool(), __PlayerInst_pool()
	, Clients(), OverlapClients(), Clients_index(0), Clients_number(0)
{
	ZeroMemory(&Overlap, sizeof(Overlap));

	InitializeCriticalSection(&Client_sect);

	PlayerInst_pool.reserve(CLIENTS_MAX_NUMBER);
	for (int i = 0; i < 10; ++i)
	{
		__PlayerInst_pool[i] = new Player;

		PlayerInst_pool.push_back(__PlayerInst_pool[i]);
	}
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

	auto worker = CreateThread(NULL, 0, Communicate, this, 0, NULL);
	if (0 == worker)
	{
		ErrorDisplay("CreateThread()");
		return;
	}

	while (true)
	{
		SleepEx(1000, TRUE);
		BroadcastWorld();
	}
}

UINT ServerFramework::GetClientsNumber() const
{
	return Clients_number;
}

void ServerFramework::AddClient(INT nid, Session* session)
{
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
	Clients.erase(nid);
}

void ServerFramework::RemoveClient(LPWSAOVERLAPPED overlap)
{
	OverlapClients.erase(overlap);
}

void ServerFramework::RemovePlayerInstance(Player* instance)
{
	PlayerInst_pool.emplace_back((instance));
}

void ServerFramework::RemoveSession(const INT id)
{
	EnterCriticalSection(&Client_sect);
	auto session = GetClient(id);

	auto inst = session->Instance;
	if (inst) RemovePlayerInstance(inst);

	RemoveClient(id);
	RemoveClient(session->Overlap_recv);
	RemoveClient(session->Overlap_send);

	delete session;
	Clients_number--;
	LeaveCriticalSection(&Client_sect);
}

void ServerFramework::AssignPlayerInstance(Player*& instance)
{
	EnterCriticalSection(&Client_sect);
	auto inst = PlayerInst_pool.back();
	PlayerInst_pool.pop_back();
	instance = (inst); //GetInstancesData(Players_pool_index);
	LeaveCriticalSection(&Client_sect);
}

Player* ServerFramework::GetInstancesData(int index)
{
	return __PlayerInst_pool[index];
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
	AddClient(Clients_index, session);
	AddClient(session->Overlap_recv, session);
	AddClient(session->Overlap_send, session);
	Clients_number++;

	session->ID = Clients_index++;

	session->ReceiveStartPosition();
	LeaveCriticalSection(&Client_sect);
}

void ServerFramework::BroadcastWorld()
{
	EnterCriticalSection(&Client_sect);
	if (0 < Clients_number)
	{
		for_each(Clients.begin(), Clients.end(), [](pair<const INT, Session*> set) {
			auto session = set.second;
			if (session)
			{
				set.second->SendWorld();
			}
		});
	}
	LeaveCriticalSection(&Client_sect);
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
