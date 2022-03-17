#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <iostream>

using namespace std;
using std::cout;

constexpr UINT BUFFSIZE = 512;
constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // Ã¢ Å©±â
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

void ErrorDisplay(const char* title);

struct Position
{
	int x, y;
};
