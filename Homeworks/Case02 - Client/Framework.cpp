#include "stdafx.h"
#include "Framework.h"
#include "Player.h"

Framework::Framework()
	: m_Player(), Socket(), Server_address(), Buffer()
	, Board_canvas(), Board_image()
	, Board_rect{ BOARD_X, BOARD_Y, BOARD_X + BOARD_W, BOARD_Y + BOARD_H }
{}

void Framework::Init(HWND window)
{
	Window = window;

	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorDisplay(L"WSAStartup()");
		return;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == Socket)
	{
		ErrorDisplay(L"WSASocket()");
		return;
	}

	auto options = FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE;
	if (SOCKET_ERROR == WSAAsyncSelect(Socket, Window, WM_SOCKET, options))
	{
		ErrorDisplay(L"WSAAsyncSelect()");
		return;
	}

	background_color = C_GREEN;

	auto hdc = GetDC(Window);
	DC_double = CreateCompatibleDC(hdc);
	Surface_double = CreateCompatibleBitmap(hdc, WND_SZ_W, WND_SZ_H);
	Draw::Attach(DC_double, Surface_double);

	Board_canvas = CreateCompatibleDC(hdc);
	Board_image = CreateCompatibleBitmap(hdc, BOARD_W, BOARD_H);
	Draw::Attach(Board_canvas, Board_image);

	bool fill_flag = false;
	auto outliner = CreatePen(PS_NULL, 1, C_BLACK);
	auto filler = CreateSolidBrush(C_WHITE);
	Draw::Attach(Board_canvas, outliner);
	Draw::Attach(Board_canvas, filler);
	Draw::Clear(Board_canvas, BOARD_W, BOARD_H, 0);
	ReleaseDC(Window, hdc);

	m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

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
	sz_Address = sizeof(Server_address);
	ZeroMemory(&Server_address, sz_Address);
	Server_address.sin_family = AF_INET;
	Server_address.sin_port = htons(6000);
	inet_pton(AF_INET, SERVER_IP, &Server_address.sin_addr);

	int result = WSAConnect(Socket, reinterpret_cast<SOCKADDR*>(&Server_address), sz_Address, NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			ErrorDisplay(L"WSAConnect()");
			return;
		}
	}
}

void WINAPI Framework::Communicate(UINT msg, WPARAM sock, LPARAM state)
{
	auto error = WSAGETSELECTERROR(state);
	if (error)
	{
		ErrorDisplay(L"WSASelection");
		closesocket(sock);
		return;
	}

	int result = 0;
	char recv_store[BUFFSIZE + 1];
	DWORD recv_size = 0;
	DWORD recv_flag = 0;
	DWORD send_size = 0;

	auto event = WSAGETSELECTEVENT(state);
	switch (event)
	{
		case FD_CONNECT:
		{
			Position player_pos{ m_Player.x, m_Player.y };
			Buffer.buf = reinterpret_cast<char*>(&player_pos);
			Buffer.len = sizeof(player_pos);

			// send 1
			result = WSASend(Socket, &Buffer, 1, &send_size, 0, NULL, NULL); 
			if (SOCKET_ERROR == result)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					ErrorDisplay(L"WSASend 1");
					return;
				}
			}
		}
		break;

		case FD_READ:
		{
			ZeroMemory(recv_store, BUFFSIZE + 1);
			Buffer.buf = recv_store;
			Buffer.len = BUFFSIZE;

			// recv 1
			result = WSARecv(Socket, &Buffer, 1, &recv_size, &recv_flag, 0, 0);
			if (SOCKET_ERROR == result)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					ErrorDisplay(L"WSARecv 1");
					break;
				}
			}

			if (0 < recv_size)
			{
				auto position = reinterpret_cast<Position*>(recv_store);

				m_Player.x = position->x;
				m_Player.y = position->y;
				InvalidateRect(NULL, &Board_rect, FALSE);
			}
		}
		break;

		case FD_WRITE:
		{
			if (0 != Lastkey)
			{
				DWORD send_size = 0;
				WPARAM key = Lastkey;
				Lastkey = 0;

				Buffer.buf = reinterpret_cast<char*>(&key);
				Buffer.len = sizeof(key);

				// send 2
				int result = WSASend(Socket, &Buffer, 1, &send_size, 0, NULL, NULL);
				if (SOCKET_ERROR == result)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						ErrorDisplay(L"WSASend 2");
						return;
					}
				}
			}
		}
		break;

		case FD_CLOSE:
		{}
		break;
	}
}

void Framework::Render(HWND window)
{
	PAINTSTRUCT ps;
	HDC surface_app = BeginPaint(window, &ps);

	Draw::Clear(DC_double, WND_SZ_W, WND_SZ_H, background_color);

	// 파이프라인
	BitBlt(DC_double, BOARD_X, BOARD_Y, BOARD_W, BOARD_H, Board_canvas, 0, 0, SRCCOPY);
	m_Player.Render(DC_double);

	// 후면 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, WND_SZ_W, WND_SZ_H
		, DC_double, 0, 0, WND_SZ_W, WND_SZ_H, SRCCOPY);

	ReleaseDC(window, surface_app);
	EndPaint(window, &ps);
}

void Framework::SendKey(WPARAM key)
{
	Lastkey = key;
	PostMessage(Window, WM_SOCKET, Socket, FD_WRITE);
}

void ErrorDisplay(const wchar_t* title)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	WCHAR wtitle[512];
	wsprintf(wtitle, L"오류: %s", title);

	MessageBox(NULL, lpMsgBuf, wtitle, MB_OK);

	LocalFree(lpMsgBuf);
}
