#pragma once
#include "stdafx.h"
#include "Network.hpp"

class PlayerCharacter
{
public:
	void Render(HDC canvas);

	PID ID;
	INT x, y;
};
