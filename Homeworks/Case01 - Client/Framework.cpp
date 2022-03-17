#include "stdafx.h"
#include "Framework.h"
#include "Player.h"

Framework::Framework()
	: m_Player(), m_Buffer()
	, Board_canvas(), Board_image()
{}

void Framework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay("WSAStartup()");
		return;
	}

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == m_Socket)
	{
		ErrorDisplay("WSASocket()");
		return;
	}

	Event_send = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (0 == Event_send)
	{
		ErrorDisplay("CreateEvent()");
		return;
	}

	background_color = C_GREEN;
	auto hdc = GetDC(NULL);
	Board_canvas = CreateCompatibleDC(hdc);
	Board_image = CreateCompatibleBitmap(hdc, BOARD_W, BOARD_H);
	Draw::Attach(Board_canvas, Board_image);

	bool fill_flag = false;
	auto outliner = CreatePen(PS_NULL, 1, C_BLACK);
	auto filler = CreateSolidBrush(C_WHITE);
	Draw::Attach(Board_canvas, outliner);
	Draw::Attach(Board_canvas, filler);
	Draw::Clear(Board_canvas, BOARD_W, BOARD_H, 0);

	for (int i = 0; i < CELLS_LENGTH; ++i)
	{
		int sort_ratio = static_cast<int>(i / CELLS_CNT_H);
		int x = (i - sort_ratio * CELLS_CNT_H) * CELL_W;
		int y = sort_ratio * CELL_H;

		if (fill_flag)
		{
			auto blk_filler = CreateSolidBrush(C_BLACK);
			auto white_filler = Draw::Attach(Board_canvas, blk_filler);
			Draw::SizedRect(Board_canvas, x, y, CELL_W, CELL_H);
			Draw::Detach(Board_canvas, white_filler, blk_filler);
		}
		else
		{
			Draw::SizedRect(Board_canvas, x, y, CELL_W, CELL_H);
		}

		if ((CELLS_CNT_H - 1) * CELL_W != x)
		{
			fill_flag = !fill_flag;
		}
	}
}

void Framework::Start()
{
	sz_Address = sizeof(m_Server_address);
	ZeroMemory(&m_Server_address, sz_Address);
	m_Server_address.sin_family = AF_INET;
	m_Server_address.sin_port = htons(6000);
	inet_pton(AF_INET, SERVER_IP, &m_Server_address.sin_addr);

	int result = WSAConnect(m_Socket, reinterpret_cast<SOCKADDR*>(&m_Server_address), sz_Address, NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		ErrorDisplay("WSAConnect()");
		return;
	}
	m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

	DWORD send_size = 0;
	Position player_pos{ m_Player.x, m_Player.y };
	m_Buffer.buf = reinterpret_cast<char*>(&player_pos);
	m_Buffer.len = sizeof(player_pos);

	result = WSASend(m_Socket, &m_Buffer, 1, &send_size, 0, NULL, NULL); // send 1
	if (SOCKET_ERROR == result)
	{
		ErrorDisplay("WSASend 1");
		return;
	}

	auto th = CreateThread(NULL, 0, &Communicate, this, 0, NULL);
	if (0 == th)
	{
		ErrorDisplay("CreateThread");
		return;
	}
}

DWORD WINAPI Communicate(PVOID param)
{
	auto framework = reinterpret_cast<Framework*>(param);
	auto& m_Socket = framework->m_Socket;
	auto& m_Player = framework->m_Player;
	auto& m_sEvent = framework->Event_send;

	int result = 0;

	WSABUF buffer{};
	char recv_store[BUFFSIZE + 1];
	DWORD recv_size = 0;
	DWORD recv_flag = 0;

	while (true)
	{
		WaitForSingleObject(m_sEvent, INFINITE);

		ZeroMemory(recv_store, BUFFSIZE + 1);
		buffer.buf = recv_store;
		buffer.len = BUFFSIZE;

		result = WSARecv(m_Socket, &buffer, 1, &recv_size, &recv_flag, 0, 0); // recv 1
		if (SOCKET_ERROR == result)
		{
			int error = WSAGetLastError();
			if (WSAEWOULDBLOCK != error)
			{
				ErrorDisplay("WSARecv 1");
				break;
			}
		}

		if (0 < recv_size)
		{
			auto position = reinterpret_cast<Position*>(recv_store);

			m_Player.x = position->x;
			m_Player.y = position->y;
			InvalidateRect(NULL, NULL, TRUE);
		}
	}

	closesocket(m_Socket);
	return 0;
}

void Framework::Update()
{}

void Framework::Render(HWND window)
{
	PAINTSTRUCT ps;
	HDC surface_app = BeginPaint(window, &ps);

	HDC surface_double = CreateCompatibleDC(surface_app);
	HBITMAP m_hBit = CreateCompatibleBitmap(surface_app, WND_SZ_W, WND_SZ_H);
	HBITMAP m_oldhBit = reinterpret_cast<HBITMAP>(Draw::Attach(surface_double, m_hBit));

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, WND_SZ_W, WND_SZ_H);
	HBITMAP m_newoldBit = reinterpret_cast<HBITMAP>(Draw::Attach(surface_back, m_newBit));

	// 초기화
	Draw::Clear(surface_double, WND_SZ_W, WND_SZ_H, background_color);

	// 파이프라인
	BitBlt(surface_double, BOARD_X, BOARD_Y, BOARD_W, BOARD_H, Board_canvas, 0, 0, SRCCOPY);
	m_Player.Render(surface_double);

	// 이중 버퍼 -> 백 버퍼
	BitBlt(surface_back, 0, 0, WND_SZ_W, WND_SZ_H, surface_double, 0, 0, SRCCOPY);
	Draw::Detach(surface_double, m_oldhBit, m_hBit);

	// 백 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, WND_SZ_W, WND_SZ_H
		, surface_back, 0, 0, WND_SZ_W, WND_SZ_H, SRCCOPY);
	Draw::Detach(surface_back, m_newoldBit, m_newBit);

	DeleteDC(surface_back);
	DeleteDC(surface_double);
	ReleaseDC(window, surface_app);

	EndPaint(window, &ps);
}

void Framework::SendKey(WPARAM key)
{
	DWORD send_size = 0;

	m_Buffer.buf = reinterpret_cast<char*>(&key);
	m_Buffer.len = sizeof(key);

	// send 2
	int result = WSASend(m_Socket, &m_Buffer, 1, &send_size, 0, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		ErrorDisplay("WSASend 2");
		return;
	}

	SetEvent(Event_send);
}

void ErrorDisplay(const char* title)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, lpMsgBuf, L"오류!", MB_OK);

	LocalFree(lpMsgBuf);
}
