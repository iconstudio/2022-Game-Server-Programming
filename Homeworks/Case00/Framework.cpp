#include "stdafx.h"
#include "Framework.h"

Framework::Framework()
	: m_Player(), m_Cells(CELLS_LENGTH)
	, Board_canvas(), Board_image()
	, BOARD_W(CELL_W * CELLS_CNT_H), BOARD_H(CELL_H * CELLS_CNT_V)
	, BOARD_X((WND_SZ_W - BOARD_W) * 0.5), BOARD_Y((WND_SZ_H - BOARD_H) * 0.5)
{}

void Framework::Start()
{
	background_color = C_GREEN;

	auto hdc = GetDC(NULL);
	Board_canvas = CreateCompatibleDC(hdc);
	Board_image = CreateCompatibleBitmap(hdc, BOARD_W, BOARD_H);
	Draw::Attach(Board_canvas, Board_image);

	bool fill_flag = false;
	auto outliner = CreatePen(PS_NULL, 1, C_BLACK); // 외곽선은 검정 색으로	
	auto filler = CreateSolidBrush(C_WHITE); // 칠은 흰색으로
	Draw::Attach(Board_canvas, outliner);
	Draw::Attach(Board_canvas, filler);
	Draw::Clear(Board_canvas, BOARD_W, BOARD_H, 0);

	m_Player.x = BOARD_X + BOARD_W * 0.5 - CELL_W * 0.5;
	m_Player.y = BOARD_Y + BOARD_H * 0.5 - CELL_H * 0.5;

	for (int i = 0; i < CELLS_LENGTH; ++i)
	{
		auto cell = std::make_shared<Cell>();

		int sort_ratio = static_cast<int>(i / CELLS_CNT_H);
		int x = (i - sort_ratio * CELLS_CNT_H) * CELL_W;
		int y = sort_ratio * CELL_H;

		if (fill_flag)
		{
			cell->color = C_BLACK;

			auto blk_filler = static_cast<HBRUSH>(CreateSolidBrush(C_BLACK));
			auto white_filler = Draw::Attach(Board_canvas, blk_filler);
			Draw::SizedRect(Board_canvas, x, y, CELL_W, CELL_H);
			Draw::Detach(Board_canvas, white_filler, blk_filler);
		}
		else
		{
			Draw::SizedRect(Board_canvas, x, y, CELL_W, CELL_H);
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
