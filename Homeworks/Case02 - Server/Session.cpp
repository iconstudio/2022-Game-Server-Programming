#include "stdafx.h"
#include "Session.h"
#include "ServerFramework.h"

Session::Session(ServerFramework* nframework, SOCKET sock)
	: Framework(nframework), Socket(sock)
	, Overlap_recv(new WSAOVERLAPPED()), Overlap_send_world(new WSAOVERLAPPED())
	, Buffer_recv(), CBuffer_recv(), Size_recv(0)
	, World_blob(), LocalWorld(), Size_send_world(0)
{
	ClearOverlap(Overlap_recv);
	ClearOverlap(Overlap_send_world);
	ClearRecvBuffer();

	Buffer_recv.buf = CBuffer_recv;
	Buffer_recv.len = BUFFSIZE;

	ZeroMemory(World_blob, sizeof(World_blob));
	WSABUF info_wbuffer{};
	info_wbuffer.buf = reinterpret_cast<char*>(&World_desc);
	info_wbuffer.len = sizeof(PacketInfo);
	World_blob[0] = move(info_wbuffer);
	WSABUF contents_wbuffer{};
	contents_wbuffer.buf = nullptr;
	contents_wbuffer.len = 0;
	World_blob[1] = move(contents_wbuffer);
}

Session::~Session()
{
	if (LocalWorld) delete LocalWorld;
}

void Session::ClearRecvBuffer()
{
	ZeroMemory(CBuffer_recv, BUFFSIZE);
	Size_recv = 0;
}

void Session::ClearOverlap(LPWSAOVERLAPPED overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

int Session::RecvPackets(LPWSABUF datas, UINT count, DWORD flags, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
{
	if (!datas) return 0;

	return WSARecv(Socket, datas, count, NULL, &flags, Overlap_recv, routine);
}

int Session::SendPackets(LPWSABUF datas, UINT count
	, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
{
	if (!datas) return 0;

	return WSASend(Socket, datas, count, NULL, 0, Overlap_send_world, routine);
}

void Session::ReceiveStartPosition(DWORD begin_bytes)
{
	cout << "클라이언트 " << ID << "에게서 시작 위치를 받아옵니다.\n";

	DWORD recv_flag = 0;

	const size_t sz_want = sizeof(Position);
	Buffer_recv.buf = (CBuffer_recv + begin_bytes);
	Buffer_recv.len = sz_want - begin_bytes;

	int result = RecvPackets(&Buffer_recv, 1, 0, CallbackStartPositions);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			Framework->RemoveSession(ID);
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
		//Instance = CreatePlayerCharacter();

		Framework->AssignPlayerInstance(Instance);
		Instance->x = positions->x;
		Instance->y = positions->y;
		cout << "플레이어 " << ID << "의 좌표: ("
			<< positions->x << ", " << positions->y << ")\n";

		ClearOverlap(Overlap_recv);
		ClearRecvBuffer();
		ReceiveKeyInput();
		Framework->BroadcastWorld();
	}
	else
	{
		cout << "클라이언트 " << ID << "에게 받아온 정보가 "
			<< sz_want - Size_recv << " 만큼 모자라서 다시 수신합니다.\n";

		ReceiveStartPosition(Size_recv);
	}
}

void Session::ReceiveKeyInput(DWORD begin_bytes)
{
	cout << "클라이언트 " << ID << "에게서 입력을 받아옵니다.\n";

	DWORD recv_flag = 0;

	const size_t sz_want = sizeof(WPARAM);
	Buffer_recv.buf = (CBuffer_recv + begin_bytes);
	Buffer_recv.len = sz_want - begin_bytes;

	int result = RecvPackets(&Buffer_recv, 1, 0, CallbackInputs);
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			Framework->RemoveSession(ID);
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

		ClearOverlap(Overlap_recv);
		ClearRecvBuffer();
		ReceiveKeyInput(0);
		Framework->BroadcastWorld();
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

void Session::GenerateWorldData()
{
	if (LocalWorld)
	{
		delete[] LocalWorld;
	}

	if (!Framework) return;

	const auto number = Framework->GetClientsNumber();
	const auto size = sizeof(Position) * number;

	LocalWorld = new Position[number]{};
	int foundindex = 0; // 메모리 오류 방지
	for (UINT i = 0; i < number; ++i)
	{
		auto inst = (Framework->GetInstancesData(i));
		if (inst)
		{
			LocalWorld[foundindex++] = *inst;
		}
	}

	World_desc.Size = size;
	World_desc.Length = number;

	auto& contents_wbuffer = World_blob[1];
	contents_wbuffer.buf = reinterpret_cast<char*>(LocalWorld);
	contents_wbuffer.len = size;
}

void Session::SendWorld(DWORD begin_bytes)
{
	cout << "클라이언트 " << ID << "에 월드 정보를 보냅니다.\n";

	if (!LocalWorld)
	{
		cout << "월드 포인터 오류\n";
		return;
	}

	int result = 0;
	if (0 == begin_bytes)
	{
		result = SendPackets(World_blob, 2, CallbackWorld);
	}
	else
	{
		//result = SendPackets(World_blob, 2, CallbackWorld);
		
		constexpr auto sz_info = sizeof(PacketInfo);

		if (begin_bytes < sz_info) // 헤더가 잘림
		{
			auto& info_wbuffer = World_blob[0];
			info_wbuffer.buf += begin_bytes;
			info_wbuffer.len -= begin_bytes;

			result = SendPackets(World_blob, 2, CallbackWorld);
		}
		else if (sz_info < begin_bytes) // 뒤에 렌더링 정보가 잘림
		{
			auto& contents_wbuffer = World_blob[1];
			contents_wbuffer.buf += begin_bytes;
			contents_wbuffer.len -= begin_bytes;

			result = SendPackets(World_blob + 1, 1, CallbackWorld);
		}
	}
	if (SOCKET_ERROR == result)
	{
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			Framework->RemoveSession(ID);
			ErrorDisplay("SendWorld()");
			return;
		}
	}
}

void Session::ProceedWorld(DWORD send_bytes)
{
	Size_send_world += send_bytes;
	const auto sz_want = World_desc.Size;
	if (0 == sz_want)
	{
		return;
	}

	if (sz_want <= Size_send_world)
	{
		ClearOverlap(Overlap_send_world);
		Size_send_world = 0;
	}
	else
	{
		cout << "클라이언트 " << ID << "에 월드 정보를 "
			<< send_bytes << " 만큼 보냈습니다.\n";

		SendWorld(Size_send_world);
	}
}
