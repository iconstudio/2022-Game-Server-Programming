#include "stdafx.h"
#include "Framework.h"

Framework::Framework() : m_Player{}, m_Cells { CELLS_LENGTH }
{}

void Framework::Start()
{
	background_color = C_GREEN;

	auto hdc = GetDC(NULL);
	Board_canvas = CreateCompatibleDC(hdc);
	Board_image = CreateCompatibleBitmap(Board_canvas, WND_SZ_W, WND_SZ_H);
	Draw::

	for (int i = 0; i < CELLS_LENGTH; ++i)
	{
		auto cell = std::make_shared<Cell>();
		if (i % 2 == 1) cell->color = C_BLACK;

		auto sort_ratio = (i / CELLS_CNT_H);
		cell->x = (i - sort_ratio) * CELL_W;
		cell->y = sort_ratio * CELL_W;

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
	HBITMAP m_oldhBit = reinterpret_cast<HBITMAP>(Attach(surface_double, m_hBit));

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, WND_SZ_W, WND_SZ_H);
	HBITMAP m_newoldBit = reinterpret_cast<HBITMAP>(Attach(surface_back, m_newBit));

	// 초기화
	Draw::Clear(surface_double, WND_SZ_W, WND_SZ_H, background_color);

	// 파이프라인
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


void Player::Update(float delta_time)
{

}

void Player::Render(HDC canvas)
{
	Draw::Rect(canvas, x, y, x + 50, y + 50);
}

HGDIOBJ Draw::Attach(HDC canvas, HGDIOBJ object)
{
	return SelectObject(canvas, object);
}

void Draw::Detach(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new)
{
	Attach(canvas, object_old);
	DeleteObject(object_new);
}

void Draw::Clear(HDC canvas, int width, int height, COLORREF color)
{
	auto m_hPen = CreatePen(PS_NULL, 1, color);
	auto m_oldhPen = static_cast<HPEN>(Attach(canvas, m_hPen));
	auto m_hBR = CreateSolidBrush(color);
	auto m_oldhBR = static_cast<HBRUSH>(Attach(canvas, m_hBR));
	Rect(canvas, 0, 0, width, height);
	Detach(canvas, m_oldhBR, m_hBR);
	Detach(canvas, m_oldhPen, m_hPen);
}

BOOL Draw::Rect(HDC canvas, int x1, int y1, int x2, int y2)
{
	return Rectangle(canvas, x1, y1, x2, y2);
}
