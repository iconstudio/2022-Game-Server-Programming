#pragma once
#include "Player.h"

struct Cell
{
	int x, y;
	COLORREF color = C_WHITE;
};

class Framework
{
public:
	Framework();

	void Start();
	void Update(float delta_time);
	void Render(HWND window);

	COLORREF background_color = C_WHITE;

private:
	Player m_Player;
	std::vector<std::shared_ptr<Cell>> m_Cells;

	HDC Board_canvas;
	HBITMAP Board_image;

	const size_t CELL_W = 32;
	const size_t CELL_H = 32;
	const size_t CELLS_CNT_H = 8;
	const size_t CELLS_CNT_V = 8;
	const size_t CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
	const size_t BOARD_W, BOARD_H, BOARD_X, BOARD_Y;
};
