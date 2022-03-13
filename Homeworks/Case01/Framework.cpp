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
	WSADATA wsa;
}

void Framework::Bind()
{}

SOCKET Framework::Accept()
{
	return SOCKET();
}

void Framework::Close()
{}
