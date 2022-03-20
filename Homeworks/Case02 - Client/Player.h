#pragma once

struct Position
{
	int x, y;
};

class Player : public Position
{
public:
	void Render(HDC canvas);
};
