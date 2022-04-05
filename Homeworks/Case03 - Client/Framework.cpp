#include "stdafx.h"
#include "Framework.h"
#include "Network.hpp"
#include "Player.h"

ClientFramework::ClientFramework()
	: Clients(), myCharacter(nullptr), Lastkey(-1)
	, Socket(), serverAddress(), serverAddressSize(0), serverIP("127.0.0.1")
	, recvOverlap(), recvBuffer(), recvCBuffer(), recvBytes(0)
	, boardSurface(), boardBitmap()
	, boardArea{ BOARD_X, BOARD_Y, BOARD_X + BOARD_W, BOARD_Y + BOARD_H }
{
	Clients.reserve(CLIENTS_MAX_NUMBER);
}

void ClientFramework::Init(HWND window)
{
	Window = window;

	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		ErrorAbort(L"WSAStartup()");
		return;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == Socket)
	{
		ErrorAbort(L"WSASocket()");
		return;
	}

	background_color = C_WHITE;

	auto hdc = GetDC(Window);
	doubleDCSurface = CreateCompatibleDC(hdc);
	doubleDCBitmap = CreateCompatibleBitmap(hdc, WND_SZ_W, WND_SZ_H);
	Draw::Attach(doubleDCSurface, doubleDCBitmap);

	boardSurface = CreateCompatibleDC(hdc);
	boardBitmap = CreateCompatibleBitmap(hdc, BOARD_W, BOARD_H);
	Draw::Attach(boardSurface, boardBitmap);

	bool fill_flag = false;
	auto outliner = CreatePen(PS_NULL, 1, C_BLACK);
	auto filler = CreateSolidBrush(C_WHITE);
	Draw::Attach(boardSurface, outliner);
	Draw::Attach(boardSurface, filler);
	Draw::Clear(boardSurface, BOARD_W, BOARD_H, 0);
	ReleaseDC(Window, hdc);

	for (int i = 0; i < CELLS_LENGTH; ++i)
	{
		int sort_ratio = static_cast<int>(i / CELLS_CNT_H);
		int x = (i - sort_ratio * CELLS_CNT_H) * CELL_W;
		int y = sort_ratio * CELL_H;

		if (fill_flag)
		{
			auto blk_filler = CreateSolidBrush(C_BLACK);
			auto white_filler = Draw::Attach(boardSurface, blk_filler);
			Draw::SizedRect(boardSurface, x, y, CELL_W, CELL_H);
			Draw::Detach(boardSurface, white_filler, blk_filler);
		}
		else
		{
			Draw::SizedRect(boardSurface, x, y, CELL_W, CELL_H);
		}

		if ((CELLS_CNT_H - 1) * CELL_W != x)
		{
			fill_flag = !fill_flag;
		}
	}
}

void ClientFramework::Start()
{
	background_color = C_GREEN;

	serverAddressSize = sizeof(serverAddress);
	ZeroMemory(&serverAddress, serverAddressSize);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

	int result = WSAConnect(Socket, reinterpret_cast<SOCKADDR*>(&serverAddress), serverAddressSize, NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			ErrorAbort(L"WSAConnect()");
			return;
		}
	}
}

void ClientFramework::Update()
{}

void CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{

}

void CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{

}

void ClientFramework::Communicate(UINT msg, WPARAM sock, LPARAM state)
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
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));

	DWORD recv_size = 0;
	DWORD recv_flag = 0;
	DWORD send_size = 0;

	auto event = WSAGETSELECTEVENT(state);
	switch (event)
	{
		case FD_CONNECT:
		{
			Buffer_send.buf = reinterpret_cast<char*>(&m_Player);
			Buffer_send.len = sizeof(m_Player);

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
			ZeroMemory(recvCBuffer, sizeof(recvCBuffer));

			const auto sz_header = sizeof(PacketInfo);
			auto& header_buffer = Buffer_world[0];
			header_buffer.buf = recvCBuffer;
			header_buffer.len = sz_header;

			auto& contents_buffer = Buffer_world[1];
			contents_buffer.buf = recvCBuffer + sz_header;
			contents_buffer.len = BUFFSIZE - sz_header;

	//m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	//m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

			// recv 1
			result = WSARecv(Socket, Buffer_world, 2, &recv_size, &recv_flag, 0, 0);
			if (SOCKET_ERROR == result)
			{
				if (WSAEWOULDBLOCK != WSAGetLastError())
				{
					ErrorDisplay(L"WSARecv 1");
					break;
				}
			}

			if (0 == recv_size)
			{
				break;
			}

			bool cleanup = false;
			if (sz_header <= recv_size)
			{
				auto info = reinterpret_cast<PacketInfo*>(recvCBuffer);

				ULONG instance_count = info->Length;
				ULONG sz_world_blob = info->Size;
				if (-1 == ID)
				{
					ID = info->Player_id; // 플레이어 ID 받아오기
					cleanup = true;
				}

				recv_size -= sz_header;
				World_instances.clear();
				World_instances.shrink_to_fit();

				if (sz_world_blob <= recv_size)
				{
					auto stride = sizeof(PlayerCharacter);
					auto& bf_world = contents_buffer.buf;
					auto& sz_world = contents_buffer.len;

					World_instances.reserve(instance_count);
					for (auto it = bf_world; it < bf_world + sz_world_blob; it += stride)
					{
						auto instance = reinterpret_cast<PlayerCharacter*>(it);

						World_instances.push_back(move(*instance));
					}

					cleanup = true;
				}
			}

			if (cleanup)
			{
				InvalidateRect(Window, NULL, FALSE);
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
					if (WSAEWOULDBLOCK != WSAGetLastError())
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

void ClientFramework::Render(HWND window)
{
	PAINTSTRUCT ps;
	HDC surface_app = BeginPaint(window, &ps);

	Draw::Clear(doubleDCSurface, WND_SZ_W, WND_SZ_H, background_color);

	// 파이프라인
	switch (Status)
	{
		case GAME_STATES::Begin:
		{
			auto filler = CreateSolidBrush(C_BLACK);
			auto old_filler = Draw::Attach(doubleDCSurface, filler);

			auto xpos = WND_SZ_W * 0.5;
			auto ypos = WND_SZ_H * 0.5;
			auto old_align = SetTextAlign(doubleDCSurface, TA_CENTER);
			constexpr auto notification = L"접속할 서버의 IP 주소를 입력해주세요.";
			auto noti_len = lstrlen(notification);

			size_t ip_size = 16;
			WCHAR ip_address[17];
			mbstowcs_s(&ip_size, ip_address, serverIP.c_str(), 16);

			TextOut(doubleDCSurface, xpos, ypos - 70, notification, noti_len);
			TextOut(doubleDCSurface, xpos, ypos, ip_address, lstrlen(ip_address));

			SetTextAlign(doubleDCSurface, old_align);

			Draw::Detach(doubleDCSurface, old_filler, filler);
		}
		break;

		case GAME_STATES::Game:
		{
			auto filler = CreateSolidBrush(C_BLACK);
			auto old_filler = Draw::Attach(doubleDCSurface, filler);
			auto old_align = SetTextAlign(doubleDCSurface, TA_LEFT);

			const size_t sz_id = 32;
			WCHAR text_id[sz_id];
			ZeroMemory(text_id, sizeof(text_id));
			wsprintf(text_id, L"플레이어 ID: %u", ID);

			TextOut(doubleDCSurface, 8, 8, text_id, lstrlen(text_id));

			SetTextAlign(doubleDCSurface, old_align);
			Draw::Detach(doubleDCSurface, old_filler, filler);

			BitBlt(doubleDCSurface, BOARD_X, BOARD_Y, BOARD_W, BOARD_H, boardSurface, 0, 0, SRCCOPY);

			for_each(Clients.begin(), Clients.end(), [&](Session* client) {
				auto character = client->Instance;
				if (character)
				{
					character->Render(doubleDCSurface);
				}
			});
		}
		break;

		default: break;
	}

	// 후면 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, WND_SZ_W, WND_SZ_H
		, doubleDCSurface, 0, 0, WND_SZ_W, WND_SZ_H, SRCCOPY);

	ReleaseDC(window, surface_app);
	EndPaint(window, &ps);
}

void ClientFramework::InputEvent(WPARAM key)
{
	switch (Status)
	{
		case GAME_STATES::Begin:
		{
			EnterIpChar(key);
		}
		break;

		case GAME_STATES::Game:
		{
			SendKey(key);
		}
		break;

		default: break;
	}
}

void ClientFramework::EnterIpChar(WPARAM key)
{
	if (key == VK_RETURN)
	{
		Status = GAME_STATES::Game;
		Start();
	}
	else if (key == VK_BACK)
	{
		if (0 < serverIP.length())
		{
			serverIP.erase(serverIP.end() - 1);
		}
	}
	else if (isdigit(key))
	{
		if (serverIP.length() < 16)
		{
			serverIP.push_back((char)(key));
		}
	}
	else if (key == VK_OEM_PERIOD)
	{
		if (serverIP.length() < 16)
		{
			serverIP.push_back('.');
		}
	}

	InvalidateRect(Window, NULL, TRUE);
}

void ClientFramework::SendKey(WPARAM key)
{
	Lastkey = key;
	PostMessage(Window, WM_SOCKET, Socket, FD_WRITE);
}
