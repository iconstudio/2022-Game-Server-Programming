#include "stdafx.h"
#include "Framework.h"

Framework::Framework() : m_Player{}, m_Cells { LENGTH }
{}

void Framework::Start()
{
	background_color = C_GREEN;

	for (int i = 0; i < LENGTH; ++i)
	{
		auto cell = std::make_shared<Cell>();
		m_Cells.push_back(std::move(cell));
	}
}

void Framework::Update(float delta_time)
{

}

void Framework::Render(HWND window)
{
	PAINTSTRUCT ps;
	HDC surface_app = BeginPaint(window, &ps);

	HDC surface_double = CreateCompatibleDC(surface_app);
	HBITMAP m_hBit = CreateCompatibleBitmap(surface_app, WND_SZ_W, WND_SZ_H);
	HBITMAP m_oldhBit = reinterpret_cast<HBITMAP>(SelectObject(surface_double, m_hBit));

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, WND_SZ_W, WND_SZ_H);
	HBITMAP m_newoldBit = reinterpret_cast<HBITMAP>(SelectObject(surface_back, m_newBit));

	// 초기화
	DrawClear(surface_back, WND_SZ_W, WND_SZ_H, background_color);
	DrawClear(surface_double, WND_SZ_W, WND_SZ_H, background_color);

	// 파이프라인
	DrawClear(surface_app, WND_SZ_W, WND_SZ_H, background_color);
	DrawRect(surface_double, 0, 0, 50, 50);

	// 이중 버퍼 -> 백 버퍼
	BitBlt(surface_back, 0, 0, WND_SZ_W, WND_SZ_H, surface_double, 0, 0, SRCCOPY);
	DrawEnd(surface_double, m_oldhBit, m_hBit);

	// 백 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, WND_SZ_W, WND_SZ_H
		, surface_back, 0, 0, WND_SZ_W, WND_SZ_H, SRCCOPY);
	DrawEnd(surface_back, m_newoldBit, m_newBit);

	DeleteDC(surface_back);
	DeleteDC(surface_double);
	ReleaseDC(window, surface_app);

	EndPaint(window, &ps);
}

void Framework::DrawEnd(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new)
{
	SelectObject(canvas, object_old);
	DeleteObject(object_new);
}

void Framework::DrawClear(HDC canvas, int width, int height, COLORREF color)
{
	auto m_hPen = CreatePen(PS_NULL, 1, color);
	auto m_oldhPen = static_cast<HPEN>(SelectObject(canvas, m_hPen));
	auto m_hBR = CreateSolidBrush(color);
	auto m_oldhBR = static_cast<HBRUSH>(SelectObject(canvas, m_hBR));
	DrawRect(canvas, 0, 0, width, height);
	DrawEnd(canvas, m_oldhBR, m_hBR);
	DrawEnd(canvas, m_oldhPen, m_hPen);
}

BOOL Framework::DrawRect(HDC canvas, int x1, int y1, int x2, int y2)
{
	return Rectangle(canvas, x1, y1, x2, y2);
}
