#pragma once

#pragma pack(push, 1)
class Player : public Position
{
public:
	void Render(HDC canvas);

	USHORT ID;
};
#pragma pack(pop)
