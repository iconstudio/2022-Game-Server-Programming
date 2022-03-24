#include "stdafx.h"
#include "Session.h"
#include "ServerFramework.h"

Session::Session(ServerFramework* nframework, SOCKET sock)
	: Framework(nframework), Socket(sock)
	, Overlap_recv(new WSAOVERLAPPED()), Overlap_send(new WSAOVERLAPPED())
	, Buffer_recv(), Buffer_send(), CBuffer_recv(), CBuffer_send()
	, Size_recv(0), Size_send(0)
{
	ClearOverlap(Overlap_recv);
	ClearOverlap(Overlap_send);
	ClearRecvBuffer();
	ClearSendBuffer();

	Framework->AddClient(Overlap_recv, this);
	Framework->AddClient(Overlap_send, this);

	Buffer_recv.buf = CBuffer_recv;
	Buffer_recv.len = BUFFSIZE;
	Buffer_send.buf = CBuffer_send;
	Buffer_send.len = BUFFSIZE;
}

Session::~Session()
{
	Framework->RemoveClient(Overlap_recv);
	Framework->RemoveClient(Overlap_send);
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(CBuffer_recv, BUFFSIZE);
	Size_recv = 0;
}

void Session::ClearSendBuffer()
{
	ZeroMemory(CBuffer_send, BUFFSIZE);
	Size_send = 0;
}

void Session::ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void Session::ReceiveStartPosition(DWORD begin_bytes)
{
	cout << "클라이언트 " << ID << "에게서 시작 위치를 받아옵니다.\n";

	DWORD recv_flag = 0;

	const size_t sz_want = sizeof(Position);
	Buffer_recv.buf = (CBuffer_recv + begin_bytes);
	Buffer_recv.len = sz_want - begin_bytes;

	int result = WSARecv(Socket, &Buffer_recv, 1
		, NULL, &recv_flag
		, Overlap_recv, CallbackStartPositions);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			ErrorDisplay("ReceiveStartPosition()");
			return;
		}
	}
}

void Session::ProceedStartPosition(DWORD recv_bytes)
{
	Size_recv += recv_bytes;
	constexpr size_t sz_want = sizeof(Position);

	if (sz_want <= Size_recv)
	{
		auto& wbuffer = Buffer_recv;
		auto& cbuffer = wbuffer.buf;
		auto& sz_recv = wbuffer.len;

		auto positions = reinterpret_cast<Position*>(cbuffer);
		Instance = CreatePlayerCharacter();
		Instance->x = positions->x;
		Instance->y = positions->y;
		cout << "플레이어 좌표: ("
			<< positions->x << ", " << positions->y << ")\n";

		ClearRecvBuffer();
		ReceiveKeyInput();
	}
	else
	{
		cout << "클라이언트 " << ID << "에게 받아온 정보가 "
			<< sz_want - Size_recv << " 만큼 모자라서 다시 수신합니다.\n";

		ReceiveStartPosition(Size_recv);
	}
}

Player* Session::CreatePlayerCharacter()
{
	return (new Player);
}

void Session::ReceiveKeyInput(DWORD begin_bytes)
{
	cout << "클라이언트 " << ID << "에게서 입력을 받아옵니다.\n";

	DWORD recv_flag = 0;

	const size_t sz_want = sizeof(WPARAM);
	Buffer_recv.buf = (CBuffer_recv + begin_bytes);
	Buffer_recv.len = sz_want - begin_bytes;

	int result = WSARecv(Socket, &Buffer_recv, 1
		, NULL, &recv_flag
		, Overlap_recv, CallbackInputs);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			ErrorDisplay("ReceiveKeyInput()");
			return;
		}
	}
}

void Session::ProceedKeyInput(DWORD recv_bytes)
{
	Size_recv += recv_bytes;
	constexpr size_t sz_want = sizeof(WPARAM);

	if (sz_want <= Size_recv)
	{
		auto& wbuffer = Buffer_recv;
		auto& cbuffer = wbuffer.buf;
		auto& sz_recv = wbuffer.len;

		WPARAM received = 0;
		memcpy_s(&received, sizeof(received), cbuffer, sz_recv);

		auto moved = TryMove(received);

		if (!moved)
		{
			cout << "플레이어 " << ID << " - 움직이지 않음.\n";
		}
		else
		{
			cout << "플레이어 " << ID << " - 위치: ("
				<< Instance->x << ", " << Instance->y << ")\n";
		}

		ClearRecvBuffer();
		ReceiveKeyInput(0);
	}
	else
	{
		cout << "클라이언트 " << ID << "에게 받아온 정보가 "
			<< sz_want - Size_recv << " 만큼 모자라서 다시 수신합니다.\n";

		ReceiveKeyInput(Size_recv);
	}
}

bool Session::TryMove(WPARAM input)
{
	bool moved = false;
	switch (input)
	{
		case VK_LEFT:
		{
			moved = Instance->TryMoveLT();
		}
		break;

		case VK_RIGHT:
		{
			moved = Instance->TryMoveRT();
		}
		break;

		case VK_UP:
		{
			moved = Instance->TryMoveUP();
		}
		break;

		case VK_DOWN:
		{
			moved = Instance->TryMoveDW();
		}
		break;

		default:
		break;
	}

	return moved;
}

void Session::SendWorld(LPWSABUF world_info, DWORD send_bytes)
{
	cout << "클라이언트 " << ID << "에 월드 정보를 보냅니다.\n";

	DWORD recv_flag = 0;

	const size_t sz_want = sizeof(WPARAM);
	Buffer_recv.buf = (CBuffer_recv + begin_bytes);
	Buffer_recv.len = sz_want - begin_bytes;

	int result = WSARecv(Socket, &Buffer_recv, 1
		, NULL, &recv_flag
		, Overlap_recv, CallbackInputs);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			ErrorDisplay("ReceiveKeyInput()");
			return;
		}
	}
}

void Session::ProceedWorld(DWORD send_bytes)
{

}
