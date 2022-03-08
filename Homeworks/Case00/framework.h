#pragma once

class Player
{
public:
	int x, y;
};

struct Cell
{
	COLORREF color = C_WHITE;
};

class Framework
{
public:
	Framework();

	void Start();
	void Update(float delta_time);
	void Render(HWND window);

	void DrawEnd(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new);
	void DrawClear(HDC canvas, int width, int height, COLORREF color);
	BOOL DrawRect(HDC canvas, int x1, int y1, int x2, int y2);

	COLORREF background_color = C_WHITE;

private:
	const size_t SZ_W = 10;
	const size_t SZ_H = 10;
	const size_t LENGTH = SZ_W * SZ_H;

	Player m_Player;
	std::vector<std::shared_ptr<Cell>> m_Cells;
};
