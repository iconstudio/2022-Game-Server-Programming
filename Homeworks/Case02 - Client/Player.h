#pragma once

struct Position
{
	USHORT x, y;
};

class PlayerCharacter : public Position
{
public:
	void Render(HDC canvas);
};
