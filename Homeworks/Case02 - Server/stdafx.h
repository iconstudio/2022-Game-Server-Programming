#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <atomic>
using namespace std;
using std::cout;

#pragma pack(push, 1)
struct PacketInfo
{
	USHORT Length = 0;
	ULONG Size = 0;
};

struct Position
{
	USHORT x, y;
};

class PlayerCharacter : public Position
{
public:
	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();

	USHORT ID;
};
#pragma pack(pop)

void CallbackStartPositions(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CallbackInputs(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CallbackWorld(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorDisplay(const char* title);

constexpr UINT CLIENTS_MAX_NUMBER = 10;
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
constexpr int BOARD_X = (WND_SZ_W - BOARD_W) / 2;
constexpr int BOARD_Y = (WND_SZ_H - BOARD_H - 20) / 2;
