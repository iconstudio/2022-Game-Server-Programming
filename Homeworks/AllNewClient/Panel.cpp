#include "pch.hpp"
#include "Panel.hpp"
#include "resource.h"

Panel::Panel(size_t cw, size_t ch)
	: width(cw), height(ch)
	, instance(NULL), hwindow(NULL), procedure(NULL)
	, properties(), title_caption(), class_id()
{}

Panel::~Panel()
{
	UnregisterClass(class_id, instance);
}

bool Panel::Initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show)
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
	properties.lpszClassName = id;
	properties.hIconSm = LoadIcon(properties.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&properties);

	DWORD window_attributes = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(id, title, window_attributes
		, CW_USEDEFAULT, 0, int(width), int(height)
		, nullptr, nullptr, instance, nullptr);

	instance = handle;
	title_caption = title;
	class_id = id;

	if (NULL == hWnd)
	{
		return false;
	}

	hwindow = hWnd;
	ShowWindow(hWnd, cmd_show);
	UpdateWindow(hWnd);

	return true;
}

HWND Panel::GetHandle() const
{
	return hwindow;
}

size_t Panel::GetWidth() const
{
	return width;
}

size_t Panel::GetHeight() const
{
	return height;
}
