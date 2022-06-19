#include "pch.hpp"
#include "PlayerCharacter.hpp"

PlayerCharacter::PlayerCharacter(PID id)
	: GameObject(id)
{}

PlayerCharacter::PlayerCharacter(PID id, float x, float y)
	: GameObject(id, x, y)
{}

PlayerCharacter::PlayerCharacter(PID id, float pos[2])
	: GameObject(id, pos[0], pos[1])
{}

void PlayerCharacter::Awake()
{}

void PlayerCharacter::Start()
{}

void PlayerCharacter::Update(float time_elapsed)
{}

bool PlayerCharacter::TryMoveLT()
{
	if (8.0f < myPosition[0])
	{
		myPosition[0] -= 16.0f;
		return true;
	}
	else if (myPosition[0] != 8.0f)
	{
		myPosition[0] = 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveRT()
{
	if (myPosition[0] < WORLD_W - 8.0f)
	{
		myPosition[0] += 16.0f;
		return true;
	}
	else if (myPosition[0] != WORLD_W - 8.0f)
	{
		myPosition[0] = WORLD_W - 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveUP()
{
	if (8.0f < myPosition[1])
	{
		myPosition[1] -= 16.0f;
		return true;
	}
	else if (myPosition[1] != 8.0f)
	{
		myPosition[1] = 8.0f;
		return true;
	}
	return false;
}

bool PlayerCharacter::TryMoveDW()
{
	if (myPosition[1] < WORLD_H - 8.0f)
	{
		myPosition[1] += 16.0f;
		return true;
	}
	else if (myPosition[1] != WORLD_H - 8.0f)
	{
		myPosition[1] = WORLD_H - 8.0f;
		return true;
	}
	return false;
}
