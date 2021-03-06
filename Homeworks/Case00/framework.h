#pragma once
#include "Player.h"

struct Cell
{
	int x = 0, y = 0;
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

	static const int BOARD_W, BOARD_H, BOARD_X, BOARD_Y;

private:
	Player m_Player;

	HDC Board_canvas;
	HBITMAP Board_image;
	std::vector<std::shared_ptr<Cell>> m_Cells;

	const int CELL_W = CELL_SIZE;
	const int CELL_H = CELL_SIZE;
	const int CELLS_CNT_H = 8;
	const int CELLS_CNT_V = 8;
	const int CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
};
