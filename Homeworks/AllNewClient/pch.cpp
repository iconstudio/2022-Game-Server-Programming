#include "pch.hpp"

wchar_t* msgBuffer = NULL;
wchar_t msgTitle[512]{};

void ErrorAbort(const wchar_t* title, const int errorcode)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);

	ZeroMemory(msgTitle, sizeof(msgTitle));
	wsprintf(msgTitle, L"오류 → %s", title);

	MessageBox(NULL, msgBuffer, msgTitle, MB_ICONERROR | MB_OK);

	LocalFree(msgBuffer);
	exit(errorcode);
}

void ErrorDisplay(const wchar_t* title, const int errorcode)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);

	ZeroMemory(msgTitle, sizeof(msgTitle));
	wsprintf(msgTitle, L"오류 → %s", title);

	MessageBox(NULL, msgBuffer, msgTitle, MB_ICONERROR | MB_OK);

	LocalFree(msgBuffer);
}

void ErrorAbort(const wchar_t* title)
{
	ErrorAbort(title, WSAGetLastError());
}

void ErrorDisplay(const wchar_t* title)
{
	ErrorDisplay(title, WSAGetLastError());
}

void Render::transform_set(HDC world, XFORM& info)
{
	SetWorldTransform(world, &info);
}

void Render::transform_set_identity(HDC world)
{
	SetWorldTransform(world, &transform_identity);
}

void Render::draw_end(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new)
{
	SelectObject(canvas, object_old);
	DeleteObject(object_new);
}

void Render::draw_clear(HDC canvas, int width, int height, COLORREF color)
{
	auto m_hPen = CreatePen(PS_NULL, 1, color);
	auto m_oldhPen = static_cast<HPEN>(SelectObject(canvas, m_hPen));
	auto m_hBR = CreateSolidBrush(color);
	auto m_oldhBR = static_cast<HBRUSH>(SelectObject(canvas, m_hBR));
	draw_rectangle(canvas, 0, 0, width, height);
	draw_end(canvas, m_oldhBR, m_hBR);
	draw_end(canvas, m_oldhPen, m_hPen);
}

BOOL Render::draw_rectangle(HDC canvas, int x1, int y1, int x2, int y2)
{
	return Rectangle(canvas, x1, y1, x2, y2);
}
