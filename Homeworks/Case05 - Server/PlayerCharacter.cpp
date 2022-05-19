#include "stdafx.hpp"
#include "PlayerCharacter.hpp"


PlayerCharacter::PlayerCharacter(PID id, float x, float y, float z)
	: GameEntity(id, x, y, z)
{}

bool PlayerCharacter::TryMoveLT()
{
	if (8.0f < myPosition.x)
	{
		myPosition.x -= 0.5f;
		return true;
	}
	else if (myPosition.x != 8.0f)
	{
		myPosition.x = 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveRT()
{
	if (myPosition.x < WORLD_W - 8.0f)
	{
		myPosition.x += 5.0f;
		return true;
	}
	else if (myPosition.x != WORLD_W - 8.0f)
	{
		myPosition.x = WORLD_W - 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveUP()
{
	if (8.0f < myPosition.y)
	{
		myPosition.y -= 0.5f;
		return true;
	}
	else if (myPosition.y != 8.0f)
	{
		myPosition.y = 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveDW()
{
	if (myPosition.y < WORLD_H - 8.0f)
	{
		myPosition.y += 5.0f;
		return true;
	}
	else if (myPosition.y != WORLD_H - 8.0f)
	{
		myPosition.y = WORLD_H - 8.0f;
		return true;
	}
	return false;
}
