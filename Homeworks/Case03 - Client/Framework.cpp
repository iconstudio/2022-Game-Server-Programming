#include "stdafx.h"
#include "Framework.h"
#include "Network.hpp"
#include "Player.h"

ClientFramework::ClientFramework()
	: Status(GAME_STATES::Begin), ID(-1), Nickname(""), myCharacter(nullptr)
	, Socket(), serverAddress(), serverAddressSize(0), serverIP("127.0.0.1")
	, recvOverlap(), recvBuffer(), recvCBuffer(), recvBytes(0)
	, boardSurface(), boardBitmap()
	, boardArea{ BOARD_X, BOARD_Y, BOARD_X + BOARD_W, BOARD_Y + BOARD_H }
	, Clients(), ClientsDict(), clientNumber(0), clientMaxNumber(0), Lastkey(-1)
{
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));

	Clients.reserve(CLIENTS_MAX_NUMBER);
	ClientsDict.reserve(CLIENTS_MAX_NUMBER);
}

ClientFramework::~ClientFramework()
{
	DeleteObject(boardSurface);
	DeleteDC(doubleDCSurface);
	DeleteObject(doubleDCBitmap);
	DeleteDC(boardSurface);
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
	auto old_outliner = Draw::Attach(boardSurface, outliner);
	auto old_filler = Draw::Attach(boardSurface, filler);
	Draw::Clear(boardSurface, BOARD_W, BOARD_H, 0);
	Draw::Detach(boardSurface, old_outliner, outliner);
	Draw::Detach(boardSurface, old_filler, filler);

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

void ClientFramework::Connect()
{
	Status = GAME_STATES::Connect;

	serverAddressSize = sizeof(serverAddress);
	ZeroMemory(&serverAddress, serverAddressSize);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

	int result = WSAConnect(Socket, reinterpret_cast<SOCKADDR*>(&serverAddress), serverAddressSize, NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorAbort(L"WSAConnect()");
			return;
		}
	}

	Start();
	SendSignInMsg();
}

void ClientFramework::Start()
{
	Recv();
}

void ClientFramework::InputEvent(WPARAM key)
{
	switch (Status)
	{
		case GAME_STATES::Begin:
		{
			InputIpChar(key);
		}
		break;

		case GAME_STATES::Connect:
		{}
		break;

		case GAME_STATES::Game:
		{
			InputKey(key);
		}
		break;

		default: break;
	}
}

void ClientFramework::InputIpChar(WPARAM key)
{
	if (key == VK_RETURN)
	{
		Connect();
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

void ClientFramework::InputKey(WPARAM key)
{
	Lastkey = key;
	SendSignKeyMsg();
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

		case GAME_STATES::Connect:
		{
			auto filler = CreateSolidBrush(C_BLACK);
			auto old_filler = Draw::Attach(doubleDCSurface, filler);

			auto xpos = WND_SZ_W * 0.5;
			auto ypos = WND_SZ_H * 0.5;
			auto old_align = SetTextAlign(doubleDCSurface, TA_CENTER);

			constexpr auto notification = L"접속 중 입니다...";
			auto noti_len = lstrlen(notification);
			TextOut(doubleDCSurface, xpos, ypos - 70, notification, noti_len);

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

			for_each(Clients.begin(), Clients.end(), [&](LocalSession* session) {
				auto character = session->Instance;
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

void ClientFramework::ProceedRecv(DWORD bytes)
{
	recvBytes += bytes;

	const auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes) // while
	{
		auto packet = reinterpret_cast<Packet*>(recvCBuffer);
		auto sz_want = packet->Size;

		if (sz_want <= recvBytes)
		{
			auto type = packet->Type;
			auto pid = packet->playerID;

			switch (type)
			{
				case PACKET_TYPES::SC_SIGNUP:
				{
					auto rp = reinterpret_cast<SCPacketSignUp*>(recvCBuffer);

					if (GAME_STATES::Connect == Status)
					{
						Status = GAME_STATES::Game;
						background_color = C_GREEN;

						ID = pid;
						InvalidateRect(Window, NULL, TRUE);
					}

					AddClient(pid);

					clientNumber = rp->usersCurrent;
					clientMaxNumber = rp->usersMax;
				}
				break;

				case PACKET_TYPES::SC_SIGNOUT:
				{
					auto rp = reinterpret_cast<SCPacketSignOut*>(recvCBuffer);
					if (0 != pid)
					{
						clientNumber = rp->usersCurrent;
						RemoveClient(pid);

						if (pid == ID)
						{
							Status = GAME_STATES::Begin;
							background_color = C_WHITE;
						}

						InvalidateRect(Window, NULL, FALSE);
					}
				}
				break;

				case PACKET_TYPES::SC_CREATE_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketCreateCharacter*>(recvCBuffer);

					auto session = GetClient(pid);
					if (session)
					{
						auto character = new PlayerCharacter;
						character->x = rp->x;
						character->y = rp->y;
						character->ID = pid;

						session->Instance = character;

						InvalidateRect(Window, NULL, FALSE);
					}
					else
					{
						ErrorAbort(L"SC_CREATE_CHARACTER: 클라이언트 찾을 수 없음!");
					}
				}
				break;

				case PACKET_TYPES::SC_MOVE_CHARACTER:
				{
					auto rp = reinterpret_cast<SCPacketMoveCharacter*>(recvCBuffer);

					auto session = GetClient(pid);
					if (session)
					{
						auto character = session->Instance;
						character->x = rp->x;
						character->y = rp->y;

						InvalidateRect(Window, NULL, FALSE);
					}
					else
					{
						ErrorAbort(L"SC_MOVE_CHARACTER: 클라이언트 찾을 수 없음!");
					}
				}
				break;
			}

			MoveMemory(recvCBuffer, recvCBuffer + recvBytes, BUFFSIZE - recvBytes);
			recvBytes -= sz_want;
		}
	}

	Recv(recvBytes);
}

void ClientFramework::ProceedSend(EXOVERLAPPED* overlap, DWORD bytes)
{
	if (overlap)
	{
		delete overlap;
	}
}

void ClientFramework::AddClient(const PID id)
{
	auto session = new LocalSession;
	session->ID = id;
	
	Clients.emplace_back(session);
	ClientsDict.try_emplace(id, session);
	//ClientsDict.insert(std::make_pair(id, session));
}

LocalSession* ClientFramework::GetClient(const PID id)
{
	return ClientsDict.at(id);
}

void ClientFramework::RemoveClient(const PID id)
{
	auto dit = ClientsDict.find(id);
	auto session = dit->second;

	if (ClientsDict.end() != dit)
	{
		delete session->Instance;
		ClientsDict.erase(dit);
	}

	auto vit = std::find(Clients.begin(), Clients.end(), session);
	if (Clients.end() != vit)
	{
		Clients.erase(vit);
	}
}

void ClientFramework::SendSignInMsg()
{
	auto packet = new CSPacketSignIn("Username");

	auto result = SendPacket(packet, packet->Size);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay(L"SendSignInMsg()");
			return;
		}
	}
}

void ClientFramework::SendSignOutMsg()
{
	auto packet = new CSPacketSignOut(ID);

	auto result = SendPacket(packet, packet->Size);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay(L"SendSignOutMsg()");
			return;
		}
	}
}

void ClientFramework::SendSignKeyMsg()
{
	if (0 != Lastkey)
	{
		auto packet = new CSPacketKeyInput(ID, Lastkey);

		auto result = SendPacket(packet, packet->Size);
		if (SOCKET_ERROR == result)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				ErrorDisplay(L"SendSignKeyMsg()");
				return;
			}
		}
	}
}

int ClientFramework::Recv(DWORD begin_bytes)
{
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = BUFFSIZE - begin_bytes;

	DWORD flags = 0;
	return WSARecv(Socket, &recvBuffer, 1, 0, &flags, &recvOverlap, CallbackRecv);
}

int ClientFramework::Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap)
{
	if (!datas || !overlap) return 0;

	return WSASend(Socket, datas, count, NULL, 0, overlap, CallbackSend);
}

int ClientFramework::SendPacket(Packet* packet, ULONG size)
{
	if (!packet || 0 == size) return 0;

	auto sendBuffer = new WSABUF;
	sendBuffer->buf = reinterpret_cast<char*>(packet);
	sendBuffer->len = size;

	auto overlap = new EXOVERLAPPED;
	overlap->SetSendBuffer(sendBuffer);

	return Send(sendBuffer, 1, overlap);
}
