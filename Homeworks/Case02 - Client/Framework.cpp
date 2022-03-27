#include "stdafx.h"
#include "Framework.h"
#include "Player.h"

Framework::Framework()
	: m_Player(), Lastkey(-1)
	, Socket(), Server_address()
	, Buffer_recv(), Buffer_send(), Buffer_world(), CBuffer_world(), World_instances()
	, Board_canvas(), Board_image()
	, Board_rect{ BOARD_X, BOARD_Y, BOARD_X + BOARD_W, BOARD_Y + BOARD_H }
	, Server_IP("127.0.0.1")
{
	World_instances.reserve(10);
}

void Framework::Init(HWND window)
{
	Window = window;

	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorAbort(L"WSAStartup()");
		return;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == Socket)
	{
		ErrorAbort(L"WSASocket()");
		return;
	}

	auto options = FD_CONNECT | FD_READ | FD_WRITE;
	if (SOCKET_ERROR == WSAAsyncSelect(Socket, Window, WM_SOCKET, options))
	{
		ErrorAbort(L"WSAAsyncSelect()");
		return;
	}

	m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

	background_color = C_WHITE;

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
	background_color = C_GREEN;

	sz_Address = sizeof(Server_address);
	ZeroMemory(&Server_address, sz_Address);
	Server_address.sin_family = AF_INET;
	Server_address.sin_port = htons(6000);
	inet_pton(AF_INET, Server_IP.c_str(), &Server_address.sin_addr);

	int result = WSAConnect(Socket, reinterpret_cast<SOCKADDR*>(&Server_address), sz_Address, NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			ErrorAbort(L"WSAConnect()");
			return;
		}
	}
}

void WINAPI Framework::Communicate(UINT msg, WPARAM sock, LPARAM state)
{
	auto error = WSAGETSELECTERROR(state);
	if (error)
	{
		WCHAR caption[32]{};
		wsprintf(caption, L"WSASelection 오류: %u", error);

		ErrorDisplay(caption);
		closesocket(sock);
		return;
	}

	int result = 0;
	char recv_store[BUFFSIZE + 1];
	ZeroMemory(recv_store, BUFFSIZE + 1);

	DWORD recv_size = 0;
	DWORD recv_flag = 0;
	DWORD send_size = 0;

	auto event = WSAGETSELECTEVENT(state);
	switch (event)
	{
		case FD_CONNECT:
		{
			auto player_pos = Position{ m_Player };
			Buffer_send.buf = reinterpret_cast<char*>(&player_pos);
			Buffer_send.len = sizeof(player_pos);

			// send 1
			result = WSASend(Socket, &Buffer_send, 1, &send_size, 0, NULL, NULL);
			if (SOCKET_ERROR == result)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					ErrorAbort(L"WSASend 1");
					return;
				}
			}
		}
		break;

		case FD_READ:
		{
			ZeroMemory(&Buffer_world, sizeof(Buffer_world));
			if (CBuffer_world) delete[] CBuffer_world;
			CBuffer_world = new char[BUFFSIZE + 1]{};
			ZeroMemory(CBuffer_world, BUFFSIZE + 1);

			const auto sz_header = sizeof(PacketInfo);
			auto& header_buffer = Buffer_world[0];
			header_buffer.buf = CBuffer_world;
			header_buffer.len = sz_header;

			auto& contents_buffer = Buffer_world[1];
			contents_buffer.buf = CBuffer_world + sz_header;
			contents_buffer.len = BUFFSIZE - sz_header;

			// recv 1
			result = WSARecv(Socket, Buffer_world, 2, &recv_size, &recv_flag, 0, 0);
			if (SOCKET_ERROR == result)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					ErrorDisplay(L"WSARecv 1");
					break;
				}
			}

			if (sz_header <= recv_size)
			{
				auto info = reinterpret_cast<PacketInfo*>(CBuffer_world);

				ULONG instance_count = info->Length;
				ULONG sz_world_blob = info->Size;
				recv_size -= sz_header;

				if (sz_world_blob <= recv_size)
				{
					auto stride = sizeof(Position);
					auto& bf_world = contents_buffer.buf;
					auto& sz_world = contents_buffer.len;

					World_instances.clear();
					for (auto it = bf_world; it < bf_world + sz_world_blob; it += stride)
					{
						auto instance = reinterpret_cast<Position*>(it);

						World_instances.push_back(move(instance));
					}
				}

				/*
				auto position = reinterpret_cast<Position*>(recv_store);

				m_Player.x = position->x;
				m_Player.y = position->y;
				InvalidateRect(NULL, &Board_rect, FALSE);
				ZeroMemory(recv_store, recv_size + 1);
				*/
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

				Buffer_send.buf = reinterpret_cast<char*>(&key);
				Buffer_send.len = sizeof(key);

				// send 2
				int result = WSASend(Socket, &Buffer_send, 1, &send_size, 0, NULL, NULL);
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
	switch (Status)
	{
		case States::Begin:
		{
			auto filler = CreateSolidBrush(C_BLACK);
			auto old_filler = Draw::Attach(DC_double, filler);

			auto xpos = WND_SZ_W * 0.5;
			auto ypos = WND_SZ_H * 0.5;
			auto old_align = SetTextAlign(DC_double, TA_CENTER);
			constexpr auto notification = L"접속할 서버의 IP 주소를 입력해주세요.";
			auto noti_len = lstrlen(notification);

			size_t ip_size = 16;
			WCHAR ip_address[17];
			mbstowcs_s(&ip_size, ip_address, Server_IP.c_str(), 16);

			TextOut(DC_double, xpos, ypos - 70, notification, noti_len);
			TextOut(DC_double, xpos, ypos, ip_address, lstrlen(ip_address));

			SetTextAlign(DC_double, old_align);

			Draw::Detach(DC_double, old_filler, filler);
		}
		break;

		case States::Game:
		{
			BitBlt(DC_double, BOARD_X, BOARD_Y, BOARD_W, BOARD_H, Board_canvas, 0, 0, SRCCOPY);

			for_each(World_instances.begin(), World_instances.end(), [&](Position* pos) {
				auto x = pos->x;
				auto y = pos->y;

				Draw::Ellipse(DC_double, x - 16, y - 16, x + 16, y + 16);
			});
			//m_Player.Render(DC_double);
		}
		break;

		default: break;
	}


	// 후면 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, WND_SZ_W, WND_SZ_H
		, DC_double, 0, 0, WND_SZ_W, WND_SZ_H, SRCCOPY);

	ReleaseDC(window, surface_app);
	EndPaint(window, &ps);
}

void Framework::InputEvent(WPARAM key)
{
	switch (Status)
	{
		case States::Begin:
		{
			EnterIpChar(key);
		}
		break;

		case States::Game:
		{
			SendKey(key);
		}
		break;

		default: break;
	}
}

void Framework::EnterIpChar(WPARAM key)
{
	if (key == VK_RETURN)
	{
		Status = States::Game;
		Start();
	}
	else if (key == VK_BACK)
	{
		if (0 < Server_IP.length())
		{
			Server_IP.erase(Server_IP.end() - 1);
		}
	}
	else if (isdigit(key))
	{
		if (Server_IP.length() < 16)
		{
			Server_IP.push_back((char)(key));
		}
	}
	else if (key == VK_OEM_PERIOD)
	{
		if (Server_IP.length() < 16)
		{
			Server_IP.push_back('.');
		}
	}

	InvalidateRect(Window, NULL, TRUE);
}

void Framework::SendKey(WPARAM key)
{
	Lastkey = key;
	PostMessage(Window, WM_SOCKET, Socket, FD_WRITE);
}
