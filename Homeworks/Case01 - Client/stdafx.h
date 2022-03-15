#pragma once
#include "targetver.h"

#pragma warning(disable: 4244)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <WS2tcpip.h>

#include <vector>
#include <memory>
#include <algorithm>

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);

namespace Draw
{
	HGDIOBJ Attach(HDC canvas, HGDIOBJ object);
	void Detach(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new);
	void Clear(HDC canvas, int width, int height, COLORREF color);
	BOOL SizedRect(HDC canvas, int x, int y, int w, int h);
	BOOL Rect(HDC canvas, int x1, int y1, int x2, int y2);
	BOOL Ellipse(HDC canvas, int x1, int y1, int x2, int y2);
}
