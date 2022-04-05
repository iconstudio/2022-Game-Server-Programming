#pragma once

#pragma pack(push, 1)
class PlayerCharacter
{
public:
	void Render(HDC canvas);

	USHORT x, y;
	ULONG ID;
};
#pragma pack(pop)
