#pragma once
#include "targetver.h"

#pragma warning(disable: 4244)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <WS2tcpip.h>

#include <string>
#include <vector>
#include <algorithm>
using namespace std;

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);

#pragma pack(push, 1)
struct PacketInfo
{
	USHORT Length = 0;
	ULONG Size = 0;
};
#pragma pack(pop)

void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);

namespace Draw
{
	HGDIOBJ Attach(HDC canvas, HGDIOBJ object);
	void Detach(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new);
	void Clear(HDC canvas, int width, int height, COLORREF color);
	BOOL SizedRect(HDC canvas, int x, int y, int w, int h);
	BOOL Rect(HDC canvas, int x1, int y1, int x2, int y2);
	BOOL Ellipse(HDC canvas, int x1, int y1, int x2, int y2);
}

constexpr UINT BUFFSIZE = 512;
constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // 창 크기
constexpr SIZE_T CELL_SIZE = 64;
constexpr int CELL_W = CELL_SIZE;
constexpr int CELL_H = CELL_SIZE;
constexpr int CELLS_CNT_H = 8;
constexpr int CELLS_CNT_V = 8;
constexpr int CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
constexpr int BOARD_W = CELL_W * CELLS_CNT_H;
constexpr int BOARD_H = CELL_H * CELLS_CNT_V;
constexpr int BOARD_X = (WND_SZ_W - BOARD_W) * 0.5;
constexpr int BOARD_Y = (WND_SZ_H - BOARD_H - 20) * 0.5;
