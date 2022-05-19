#pragma once
#include "GameEntity.hpp"

class PlayerCharacter : public GameEntity
{
public:
	PlayerCharacter(PID id, float x, float y, float z);

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
