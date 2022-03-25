#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework::ServerFramework()
	: Overlap()
	, World(), World_data(nullptr), World_cbuffer()
	, World_data_length(0)
	, Size_send(0)
{
	ZeroMemory(&Overlap, sizeof(Overlap));

	Clients.reserve(CLIENTS_MAX_NUMBER);
	OverlapClients.reserve(CLIENTS_MAX_NUMBER);

	World_data_info.Length = 0;
	World_data_info.Size = 0;

	World.reserve(CLIENTS_MAX_NUMBER);
	WSABUF info_buffer{};
	info_buffer.buf = reinterpret_cast<char*>(&World_data_info);
	info_buffer.len = sizeof(PacketInfo);
	World.emplace_back(move(info_buffer));
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

void ServerFramework::AddPlayerSpace(Player* instance)
{
	WSABUF wbuffer{};
	wbuffer.buf = reinterpret_cast<char*>(static_cast<Position*>(instance));
	wbuffer.len = sizeof(Position);

	World.emplace_back(move(wbuffer));
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
}

void ServerFramework::BroadcastWorld()
{
	for (auto it = Clients.begin(); it != Clients.end(); ++it)
	{
		auto session = (*it).second;
		if (session)
		{
			SendWorld(session);
		}
	}
}

void ServerFramework::SendWorld(Session* session, DWORD begin_bytes)
{
	cout << "클라이언트 " << session->ID << "에 월드 정보를 보냅니다.\n";

	const auto cdata = World.data();
	auto number = GetClientsNumber();
	auto count = number + 1;
	auto size = sizeof(WSABUF) * count;

	World_data_info.Size = size;
	World_data_info.Length = number;

	World_data = new WSABUF[count]{};
	ZeroMemory(World_data, size);

	auto seek = World_data + 1;
	for (auto it = World.begin(); it != World.end(); ++it, seek++)
	{
		*seek = *it;
	}
	//copy(World.begin(), World.end(), World_data + 1);

	int result = session->SendPackets(cdata, count, CallbackWorld);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			ErrorDisplay("SendWorld()");
			return;
		}
	}
}

void ServerFramework::ProceedWorld(Session* session, DWORD send_bytes)
{
	auto& Size_send = session->Size_send;
	Size_send += send_bytes;
	constexpr size_t sz_want = sizeof(Position);

	if (sz_want <= Size_send)
	{
		SendWorld(session, 0);
	}
	else
	{
		cout << "클라이언트 " << session->ID << "에 월드 정보를 "
			<< sz_want - Size_send << " 만큼 더 보냅니다.\n";

		SendWorld(session, Size_send);
	}
	SleepEx(100, TRUE);
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
