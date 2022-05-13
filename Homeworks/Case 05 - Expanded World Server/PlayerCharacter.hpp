#pragma once
#include "stdafx.hpp"
#include "GameObject.hpp"

class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter(CHAR ox, CHAR oy);

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();

	CHAR x, y;
};
