#include "stdafx.h"
#include "Client.h"
#include "WindowsForm.h"
#include "Framework.h"
#include "Network.hpp"

#define MAX_LOADSTRING 100
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

WindowsForm window{ WND_SZ_W, WND_SZ_H };
ClientFramework framework{};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_APP_ID, szWindowClass, MAX_LOADSTRING);

	if (!window.Initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow))
	{
		return FALSE;
	}

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
			framework.Init(hWnd);
		}
		break;

		case WM_SOCKET:
		{
			framework.Communicate(message, wParam, lParam);
		}
		break;

		case WM_KEYDOWN:
		{
			framework.InputEvent(wParam);
		}
		break;

		case WM_TIMER:
		{
			//InvalidateRect(hWnd, NULL, FALSE);
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
