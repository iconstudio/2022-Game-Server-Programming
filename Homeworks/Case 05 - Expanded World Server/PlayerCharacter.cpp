#include "stdafx.hpp"
#include "PlayerCharacter.hpp"
#include "Network.hpp"

PlayerCharacter::PlayerCharacter(CHAR ox, CHAR oy) : x(ox), y(oy)
{}

bool PlayerCharacter::TryMoveLT()
{
	if (0 < x)
	{
		x--;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveRT()
{
	if (x < WORLD_CELLS_CNT_H - 1)
	{
		x++;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveUP()
{
	if (0 < y)
	{
		y--;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveDW()
{
	if (y < WORLD_CELLS_CNT_V - 1)
	{
		y++;
		return true;
	}
	return false;
}
