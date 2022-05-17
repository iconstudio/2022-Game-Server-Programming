#pragma once
#include "GameEntity.hpp"

class PlayerCharacter : public GameEntity
{
public:
	PlayerCharacter(int ox, int oy);

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
