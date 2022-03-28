#include "stdafx.h"
#include "Resource.h"
#include "WindowsForm.h"

WindowsForm::WindowsForm(LONG cw, LONG ch)
	: width(cw), height(ch), hwindow(NULL), procedure(NULL)
{}

WindowsForm::~WindowsForm()
{
	UnregisterClassW(class_id, instance);
}

BOOL WindowsForm::Initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show)
{
	properties.cbSize = sizeof(WNDCLASSEX);
	properties.style = CS_HREDRAW | CS_VREDRAW;
	properties.lpfnWndProc = procedure;
	properties.cbClsExtra = 0;
	properties.cbWndExtra = 0;
	properties.hInstance = handle;
	properties.hIcon = LoadIcon(handle, MAKEINTRESOURCE(IDI_LICON));
	properties.hCursor = LoadCursor(nullptr, IDC_ARROW);
	properties.hbrBackground = CreateSolidBrush(0);
	properties.lpszMenuName = NULL;
	properties.lpszClassName = reinterpret_cast<LPCWSTR>(id);
	properties.hIconSm = LoadIcon(properties.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&properties);

	DWORD window_attributes = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(reinterpret_cast<LPCWSTR>(id), reinterpret_cast<LPCWSTR>(title), window_attributes
		, CW_USEDEFAULT, 0, width, height, nullptr, nullptr, instance, nullptr);
	
	instance = handle;
	title_caption = title;
	class_id = id;

	if (!hWnd)
	{
		return FALSE;
	}

	hwindow = hWnd;
	ShowWindow(hWnd, cmd_show);
	UpdateWindow(hWnd);

	return TRUE;
}

LONG WindowsForm::GetWidth() const
{
	return width;
}

LONG WindowsForm::GetHeight() const
{
	return height;
}
