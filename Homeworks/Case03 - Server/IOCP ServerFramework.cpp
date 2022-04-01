#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: cbufferAccept()
	, portOverlap(new WSAOVERLAPPED), portBytes(0), portKey(0), serverKey(100)
	, socketPool(), clientsID(), Clients()
	, orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0)
	, overlapRecv(), szRecv(0), szWantRecv(0), bufferRecv(), cbufferRecv()
{
	ClearOverlap(&overlapRecv);
	ZeroMemory(cbufferAccept, sizeof(cbufferAccept));
	ZeroMemory(&bufferRecv, sizeof(bufferRecv));
	ZeroMemory(cbufferRecv, sizeof(cbufferRecv));
	bufferRecv.buf = cbufferRecv;
	bufferRecv.len = sizeof(cbufferRecv);

	socketPool.reserve(CLIENTS_MAX_NUMBER);
	clientsID.reserve(CLIENTS_MAX_NUMBER);

	for (UINT i = 0; i < CLIENTS_MAX_NUMBER; ++i)
	{
		auto sk = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == sk)
		{
			ErrorDisplay("IOCPFramework → WSASocket()");
			return;
		}

		socketPool.push_back(move(sk));
	}
}

IOCPFramework::~IOCPFramework()
{
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

	Listener = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, NULL, 0, WSA_FLAG_OVERLAPPED);
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
}

void IOCPFramework::Start()
{
	if (SOCKET_ERROR == listen(Listener, CLIENTS_MAX_NUMBER))
	{
		ErrorDisplay("listen()");
		return;
	}

	cout << "서버 시작\n";
	while (true)
	{
		if (!Accept()) break;
		if (!Update()) break;
	}
	cout << "서버 종료\n";
}

bool IOCPFramework::Accept()
{
	auto newbie = socketPool.back();

	DWORD byte_recv = 0;
	auto result = AcceptEx(Listener, newbie, cbufferAccept
		, 0, sizeof(SOCKADDR_IN) + 16, szAddress + 16, &byte_recv
		, portOverlap);

	if (FALSE == result)
	{
		auto error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			ErrorDisplay("AcceptEx()");
			return false;
		}
	}
	return true;
}

bool IOCPFramework::Update()
{
	auto result = GetQueuedCompletionStatus(completionPort, &portBytes, &portKey, &portOverlap, INFINITE);
	if (TRUE == result)
	{
		auto byte = portBytes;
		auto key = portKey;

		if (serverKey == key) // AcceptEx
		{
			auto newbie = socketPool.back();

			auto result = CreateAndAssignClient(newbie);

			socketPool.pop_back();
			//GetAcceptExSockaddrs(0,);
			ZeroMemory(cbufferAccept, sizeof(cbufferAccept));
		}
		else
		{
			auto client = GetClient(key);
			if (!client)
			{
				//
			}
		}

		return true;
	}
	else
	{
		ErrorDisplay("GetQueuedCompletionStatus()");
		return false;
	}
}

pair<PID, Session*> IOCPFramework::CreateAndAssignClient(SOCKET nsocket)
{
	PID nid;
	auto session = new Session(orderClientIDs, nsocket, *this);
	if (!session)
	{
		throw std::exception("서버 메모리 부족!");
	}

	auto result = make_pair(nid, session);
	clientsID.push_back(orderClientIDs);
	Clients.insert(result);
	orderClientIDs++;

	return result;
}

Session* IOCPFramework::GetClient(PID id)
{
	return nullptr;
}

Session* IOCPFramework::GetClientByIndex(UINT index)
{
	return nullptr;
}

UINT IOCPFramework::GetClientsNumber() const
{
	return numberClients;
}

void IOCPFramework::RemoveClient(PID rid)
{

}

void IOCPFramework::RemoveSession(const PID id)
{

}
