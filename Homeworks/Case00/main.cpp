#include "stdafx.h"
#include "main.h"
#include "WindowsForm.h"
#include "Framework.h"

#define MAX_LOADSTRING 100
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

WindowsForm window{ WND_SZ_W, WND_SZ_H, 100 };
Framework framework{};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CASE00, szWindowClass, MAX_LOADSTRING);

	if (!window.Initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow))
	{
		return FALSE;
	}

	framework.Start();

	MSG msg;
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			framework.Start();
		}
		break;

		case WM_TIMER:
		{
			framework.Update(window.GetFrametime());
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

		case WM_PAINT:
		{
			framework.Render(hWnd);
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
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
