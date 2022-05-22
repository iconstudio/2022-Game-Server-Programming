#include "pch.hpp"
#include "stdafx.hpp"
#include "PlayerCharacter.hpp"

PlayerCharacter::PlayerCharacter(PID id, float x, float y, float z)
	: PlayerCharacter(id, XMFLOAT3(x, y, z))
{}

PlayerCharacter::PlayerCharacter(PID id, const XMFLOAT3& pos)
	: PlayerCharacter(id, XMFLOAT3(pos))
{}

PlayerCharacter::PlayerCharacter(PID id, XMFLOAT3&& pos)
	: GameEntity(id, std::forward<XMFLOAT3>(pos))
{}

void PlayerCharacter::Awake()
{}

void PlayerCharacter::Start()
{}

void PlayerCharacter::Update(float time_elapsed)
{}

bool PlayerCharacter::TryMoveLT()
{
	if (8.0f < myPosition.x)
	{
		myPosition.x -= 5.0f;
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
		myPosition.y -= 5.0f;
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
