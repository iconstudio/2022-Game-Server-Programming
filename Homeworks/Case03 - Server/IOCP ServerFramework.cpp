#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Network.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: acceptOverlap(), acceptBytes(0), acceptCBuffer()
	, portOverlap(), portBytes(0), portKey(0), serverKey(100)
	, socketPool(), clientsID(), Clients()
	, orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0)
	, recvOverlap(), szRecv(0), szWantRecv(0), bufferRecv(), cbufferRecv()
{
	ClearOverlap(&acceptOverlap);
	ClearOverlap(&recvOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
	ZeroMemory(&bufferRecv, sizeof(bufferRecv));
	ZeroMemory(cbufferRecv, sizeof(cbufferRecv));
	bufferRecv.buf = cbufferRecv;
	bufferRecv.len = sizeof(cbufferRecv);

	socketPool.reserve(CLIENTS_MAX_NUMBER);
	clientsID.reserve(CLIENTS_MAX_NUMBER);
}

IOCPFramework::~IOCPFramework()
{
	CloseHandle(completionPort);
	closesocket(Listener);
	WSACleanup();
}

void IOCPFramework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	Listener = CreateSocket();
	if (INVALID_SOCKET == Listener)
	{
		ErrorDisplay("Init → WSASocket()");
		return;
	}

	szAddress = sizeof(Address);
	ZeroMemory(&Address, szAddress);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = htonl(INADDR_ANY);
	Address.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(Listener, (SOCKADDR*)(&Address), szAddress))
	{
		ErrorDisplay("bind()");
		return;
	}

	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort)
	{
		ErrorDisplay("CreateIoCompletionPort(INVALID_HANDLE_VALUE)");
		return;
	}

	auto apply = CreateIoCompletionPort((HANDLE)(Listener)
		, completionPort, serverKey, 0);
	if (NULL == apply)
	{
		ErrorDisplay("CreateIoCompletionPort(Listener)");
		return;
	}

	for (UINT i = 0; i < socketPool.capacity(); ++i)
	{
		auto sk = CreateSocket();
		if (INVALID_SOCKET == sk)
		{
			ErrorDisplay("IOCPFramework → WSASocket()");
			return;
		}

		socketPool.push_back(std::move(sk));
	}
}

void IOCPFramework::Start()
{
	if (SOCKET_ERROR == listen(Listener, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	std::cout << "서버 시작\n";
	while (true)
	{
		Accept();
		if (!Update()) break;
	}
	std::cout << "서버 종료\n";
}

void IOCPFramework::Accept()
{
	auto newbie = socketPool.back();

	auto result = AcceptEx(Listener, newbie, acceptCBuffer
		, 0, sizeof(SOCKADDR_IN) + 16, szAddress + 16, &acceptBytes
		, &acceptOverlap);

	if (FALSE == result)
	{
		auto error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			ClearOverlap(&acceptOverlap);
			ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
			ErrorDisplay("AcceptEx()");
		}
	}
}

bool IOCPFramework::Update()
{
	auto result = GetQueuedCompletionStatus(completionPort, &portBytes, &portKey, &portOverlap, INFINITE);
	if (TRUE == result)
	{
		auto bytes = portBytes;
		auto key = portKey;

		std::cout << "GQCS: " << key << ", Bytes: " << bytes << "\n";
		if (0 == bytes)
		{
			ErrorDisplay("GetQueuedCompletionStatus()");
			return true;
		}

		if (serverKey == key) // AcceptEx
		{
			ProceedAccept();
		}
		else // recv / send
		{
			ProceedPacket(portOverlap, key, bytes);
		}

		return true;
	}
	else
	{
		ErrorDisplay("GetQueuedCompletionStatus()");
		return false;
	}
}

void IOCPFramework::ProceedAccept()
{
	auto& newbie = socketPool.back();

	if (socketPool.empty())
	{
		std::cout << "새 접속을 받을 수 없습니다!\n";
		closesocket(newbie);
		newbie = CreateSocket();
	}
	else
	{
		CreateAndAssignClient(newbie);
		socketPool.pop_back();
	}

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));
}

void IOCPFramework::ProceedPacket(LPWSAOVERLAPPED overlap, ULONG_PTR key, DWORD bytes)
{
	auto client = GetClient(PID(key));
	if (!client) // 작업은 완료됐으나 클라이언트가 없다.
	{
		delete overlap;
	}
	else
	{
		auto exoverlap = static_cast<EXOVERLAPPED*>(overlap);
		client->RoutePacket(exoverlap, bytes);
	}
}

SOCKET IOCPFramework::CreateSocket() const
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void IOCPFramework::CreateAndAssignClient(SOCKET nsocket)
{
	auto session = new Session(orderClientIDs, nsocket, *this);
	if (!session)
	{
		throw std::exception("서버 메모리 부족!");
		return;
	}

	auto io = CreateIoCompletionPort((HANDLE)(nsocket), completionPort, orderClientIDs, 0);
	if (NULL == io)
	{
		ErrorDisplay("CreateAndAssignClient → CreateIoCompletionPort()");
		return;
	}

	std::cout << "클라이언트 " << orderClientIDs << " 접속 → 소켓: " << nsocket << "\n";

	auto result = std::make_pair(orderClientIDs, session);
	clientsID.push_back(orderClientIDs);
	Clients.insert(result);
	orderClientIDs++;
	numberClients++;

	session->ReceiveSignIn(); // IO 진입
}

Session* IOCPFramework::GetClient(PID id)
{
	return Clients[id];
}

Session* IOCPFramework::GetClientByIndex(UINT index)
{
	return Clients[clientsID[index]];
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
}

void IOCPFramework::RemoveClient(const PID rid)
{
	auto vit = std::find(clientsID.begin(), clientsID.end(), rid);
	if (clientsID.end() != vit)
	{
		clientsID.erase(vit);
	}

	auto mit = Clients.find(rid);
	if (Clients.end() != mit)
	{
		Clients.unsafe_erase(mit);
		numberClients--;
	}
}

void IOCPFramework::Disconnect(const PID id)
{

}
