#include "stdafx.h"
#include "PlayerCharacter.hpp"

PlayerCharacter::PlayerCharacter(int ox, int oy)
	: GameEntity(-1, ox, oy)
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
	if (x < CELLS_CNT_H - 1)
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
	if (y < CELLS_CNT_V - 1)
	{
		y++;
		return true;
	}
	return false;
}
