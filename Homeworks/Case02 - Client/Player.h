#pragma once

#pragma pack(push, 1)
struct Position
{
	USHORT x, y;
};

class PlayerCharacter : public Position
{
public:
	void Render(HDC canvas);

	USHORT ID;
};
#pragma pack(pop)
