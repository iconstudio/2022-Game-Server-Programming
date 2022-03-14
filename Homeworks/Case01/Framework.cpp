#include "stdafx.h"
#include "Framework.h"

Framework::Framework()
	: m_Player(), m_Cells(CELLS_LENGTH)
{}

void Framework::Start()
{
	background_color = C_GREEN;

	bool fill_flag = false;
	m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

	// 보드판 만들기
	for (int i = 0; i < CELLS_LENGTH; ++i)
	{
		auto cell = std::make_shared<Cell>();

		int sort_ratio = static_cast<int>(i / CELLS_CNT_H);
		int x = (i - sort_ratio * CELLS_CNT_H) * CELL_W;
		int y = sort_ratio * CELL_H;

		if (fill_flag)
		{
			cell->color = C_BLACK;
		}

		if ((CELLS_CNT_H - 1) * CELL_W != x) // 마지막 칸
		{
			fill_flag = !fill_flag;
		}

		cell->x = x;
		cell->y = y;
		m_Cells.push_back(std::move(cell));
	}
}

void Framework::Update(float delta_time)
{
	m_Player.Update(delta_time);
}

void Framework::Render(HWND window)
{
}

void Framework::Init()
{
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		//ErrorAbort("WSAStartup()");
		return;
	}
	
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_Socket)
	{
		//ErrorAbort("socket()");
		return;
	}

	BOOL option = TRUE;
	if (SOCKET_ERROR == setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR
		, reinterpret_cast<char*>(&option), sizeof(option)))
	{
		//ErrorAbort("setsockopt()");
		return;
	}

	sz_Address = sizeof(m_Address);
	ZeroMemory(&m_Address, sz_Address);
	m_Address.sin_family = AF_INET;
	m_Address.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Address.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(m_Socket, reinterpret_cast<SOCKADDR*>(&m_Address), sz_Address))
	{
		//ErrorAbort("bind()");
		return;
	}

	if (SOCKET_ERROR == listen(m_Socket, SOMAXCONN))
	{
		//ErrorAbort("listen()");
		return;
	}

}

void Framework::Bind()
{}

SOCKET Framework::Accept()
{
	return SOCKET();
}

void Framework::Close()
{}
