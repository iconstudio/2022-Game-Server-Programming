#pragma once
#include "stdafx.h"
#include "Network.hpp"

class PlayerCharacter
{
public:
	void Render(HDC canvas, INT ax, INT ay);

	PID ID;
	INT x, y;
};
