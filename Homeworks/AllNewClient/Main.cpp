#include "pch.hpp"
#include "Main.hpp"
#include "Panel.hpp"
#include "Framework.hpp"
#include "Network.hpp"

LRESULT CALLBACK myProcedure(HWND, UINT, WPARAM, LPARAM);

#define MAX_LOADSTRING 100
wchar_t myTitle[MAX_LOADSTRING];
wchar_t myWindowClass[MAX_LOADSTRING];

Panel myWindow{ 800, 600 };
Framework myFramework{};
Network myNetwork{5000};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, myTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_APP_ID, myWindowClass, MAX_LOADSTRING);

	if (!myWindow.Initialize(hInstance, myProcedure, myTitle, myWindowClass, nCmdShow))
	{
		return FALSE;
	}

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
			{
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			SleepEx(10, TRUE);
		}
	}

	return static_cast<int>(msg.wParam);
}

void CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{
	if (0 != error || 0 == bytes)
	{
		int code = WSAGetLastError();
		if (WSA_IO_PENDING != code)
		{
			ErrorDisplay(L"CallbackRecv()", code);
		}
	}

	if (const auto& result = myNetwork.OnReceive(bytes); result)
	{
		myFramework.OnNetwork(*result);
	}
}

void CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags)
{
	if (0 != error || 0 == bytes)
	{
		int code = WSAGetLastError();
		if (WSA_IO_PENDING != code)
		{
			ErrorDisplay(L"CallbackSend()", code);
		}
	}

	myNetwork.OnSend(overlap, bytes);
}

LRESULT myProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			myFramework.Awake();
		}
		break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			myFramework.OnMouse(wparam, lparam);
		}
		break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			myFramework.OnKeyboard(wparam, lparam);
		}
		break;

		case WM_PAINT:
		{
			myFramework.BeginRendering(hwnd);
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

		default:
		{
			return DefWindowProc(hwnd, message, wparam, lparam);
		}
	}

	return 0;
}
