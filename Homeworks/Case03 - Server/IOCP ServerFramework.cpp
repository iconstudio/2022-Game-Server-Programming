#include "stdafx.h"
#include "IOCP ServerFramework.hpp"
#include "Session.h"

IOCPFramework::IOCPFramework()
	: overlapAccept(), bytesAccept(0), cbufferAccept()
	, portOverlap(), portBytes(0), portKey(0), serverKey(100)
	, socketPool(), clientsID(), Clients()
	, orderClientIDs(CLIENTS_ORDER_BEGIN), numberClients(0)
	, overlapRecv(), szRecv(0), szWantRecv(0), bufferRecv(), cbufferRecv()
{
	ClearOverlap(&overlapAccept);
	ClearOverlap(&overlapRecv);
	ZeroMemory(cbufferAccept, sizeof(cbufferAccept));
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
		Accept();
		//if (!Accept()) break;
		if (!Update()) break;
	}
	cout << "서버 종료\n";
}

void IOCPFramework::Accept()
{
	auto newbie = socketPool.back();

	auto result = AcceptEx(Listener, newbie, cbufferAccept
		, 0, sizeof(SOCKADDR_IN) + 16, szAddress + 16, &bytesAccept
		, &overlapAccept);

	if (FALSE == result)
	{
		auto error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			ClearOverlap(&overlapAccept);
			ZeroMemory(cbufferAccept, sizeof(cbufferAccept));
			ErrorDisplay("AcceptEx()");
		}
	}
}

bool IOCPFramework::Update()
{
	auto result = GetQueuedCompletionStatus(completionPort, &portBytes, &portKey, &portOverlap, INFINITE);
	if (TRUE == result)
	{
		auto byte = portBytes;
		auto key = portKey;

		cout << "GQCS: " << key << ", Bytes: " << byte << "\n";

		if (serverKey == key) // AcceptEx
		{
			auto newbie = socketPool.back();
			CreateAndAssignClient(newbie);
			socketPool.pop_back();

			ClearOverlap(&overlapAccept);
			ZeroMemory(cbufferAccept, sizeof(cbufferAccept));
		}
		else
		{
			auto overlap = static_cast<EXOVERLAPPED*>(portOverlap);
			auto op = overlap->Operation;

			switch (op)
			{
				case OVERLAP_OPS::NONE:
				{}
				break;

				case OVERLAP_OPS::RECV:
				{}
				break;

				case OVERLAP_OPS::SEND:
				{}
				break;
			}

			if (OVERLAP_OPS::NONE != op)
			{
				auto client = GetClient(PID(key));
				if (!client)
				{
				}


			}

			ClearOverlap(portOverlap);
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
	auto session = new Session(orderClientIDs, nsocket, *this);
	if (!session)
	{
		throw std::exception("서버 메모리 부족!");
		return make_pair(0, nullptr);
	}

	auto io = CreateIoCompletionPort((HANDLE)(nsocket), completionPort, orderClientIDs, 0);
	if (NULL == io)
	{
		ErrorDisplay("CreateAndAssignClient → CreateIoCompletionPort()");
		return make_pair(0, nullptr);
	}

	cout << "클라이언트 " << orderClientIDs << " 접속 → 소켓: " << nsocket << "\n";

	auto result = make_pair(orderClientIDs, session);
	clientsID.push_back(orderClientIDs);
	Clients.insert(result);
	orderClientIDs++;
	numberClients++;

	return result;
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

void IOCPFramework::RemoveClient(PID rid)
{

}

void IOCPFramework::Disconnect(const PID id)
{

}
