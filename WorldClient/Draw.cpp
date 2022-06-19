#include "pch.hpp"
#include "stdafx.hpp"
#include "Draw.hpp"

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
	SizedRect(canvas, 0, 0, width, height);
	Detach(canvas, m_oldhBR, m_hBR);
	Detach(canvas, m_oldhPen, m_hPen);
}

BOOL Draw::SizedRect(HDC canvas, int x, int y, int w, int h)
{
	return ::Rectangle(canvas, x, y, x + w, y + h);
}

BOOL Draw::Rect(HDC canvas, int x1, int y1, int x2, int y2)
{
	return ::Rectangle(canvas, x1, y1, x2, y2);
}

BOOL Draw::Ellipse(HDC canvas, int x1, int y1, int x2, int y2)
{
	return ::Ellipse(canvas, x1, y1, x2, y2);
}

void Draw::Transform(HDC world, const XFORM& info)
{
	SetWorldTransform(world, &info);
}

void Draw::ResetTransform(HDC world)
{
	SetWorldTransform(world, &transform_identity);
}
