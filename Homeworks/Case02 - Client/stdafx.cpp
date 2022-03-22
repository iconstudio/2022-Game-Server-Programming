#include "stdafx.h"

void ErrorAbort(const wchar_t* title)
{
	WCHAR* lpMsgBuf;
	auto error = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	WCHAR wtitle[512];
	wsprintf(wtitle, L"����: %s", title);

	MessageBox(NULL, lpMsgBuf, wtitle, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	exit(error);
}

void ErrorDisplay(const wchar_t* title)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	WCHAR wtitle[512];
	wsprintf(wtitle, L"����: %s", title);

	MessageBox(NULL, lpMsgBuf, wtitle, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
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
