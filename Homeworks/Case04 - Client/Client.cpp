#include "stdafx.hpp"
#include "Client.h"
#include "WindowsForm.h"
#include "Framework.hpp"
#include "Network.hpp"

#define MAX_LOADSTRING 100
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

WindowsForm Window{ WND_SZ_W, WND_SZ_H };
ClientFramework Framework{};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_APP_ID, szWindowClass, MAX_LOADSTRING);

	if (!Window.Initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow))
	{
		return FALSE;
	}

	auto msg = new MSG;
	while (true)
	{
		if (::PeekMessage(msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg->message == WM_QUIT)
			{
				break;
			}

			::TranslateMessage(msg);
			::DispatchMessage(msg);
		}
		else
		{
			SleepEx(10, TRUE);
		}
	}

	return static_cast<int>(msg->wParam);
}

void CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{
	if (0 != error || 0 == bytes)
	{
		int code = WSAGetLastError();
		if (WSA_IO_PENDING != code)
		{
			ErrorDisplay(L"CallbackRecv()");
		}
	}

	Framework.ProceedRecv(bytes);
}

void CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{
	if (0 != error || 0 == bytes)
	{
		int code = WSAGetLastError();
		if (WSA_IO_PENDING != code)
		{
			ErrorDisplay(L"CallbackSend()");
		}
	}

	auto exoverlap = static_cast<EXOVERLAPPED*>(overlap);
	Framework.ProceedSend(exoverlap, bytes);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			Framework.Init(hWnd);
		}
		break;

		case WM_KEYDOWN:
		{
			Framework.InputEvent(wParam);
		}
		break;

		case WM_PAINT:
		{
			Framework.Render(hWnd);
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
