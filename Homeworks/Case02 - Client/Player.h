#pragma once

struct Position
{
	USHORT x, y;
};

class Player : public Position
{
public:
	void Render(HDC canvas);
};
