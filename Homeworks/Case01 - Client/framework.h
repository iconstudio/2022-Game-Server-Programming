#pragma once
#include "Player.h"

void ErrorDisplay(const char* title);
DWORD WINAPI Communicate(PVOID param);

enum class States : int
{
	Begin = 0,
	Game = 1
};

class Framework
{
public:
	Framework();

	void Init(HWND window);
	void Start();
	friend DWORD WINAPI Communicate(PVOID param);
	void Render(HWND window);
	void InputEvent(WPARAM key);
	void EnterIpChar(WPARAM key);
	void SendKey(WPARAM key);

	COLORREF background_color = C_WHITE;

private:
	States Status = States::Begin;
	Player m_Player;

	HWND Window;
	HDC DC_double;
	HBITMAP Surface_double;
	HDC Board_canvas;
	HBITMAP Board_image;
	RECT Board_rect;

	SOCKET m_Socket;
	SOCKADDR_IN m_Server_address;
	INT sz_Address;
	WSABUF m_Buffer;
	HANDLE Event_send;

	std::string SERVER_IP;
};

struct Position
{
	int x, y;
};

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
