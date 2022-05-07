#pragma once
#include "stdafx.hpp"
#include "Network.hpp"

class PlayerCharacter
{
public:
	void Render(HDC canvas);

	PID ID;
	UCHAR x, y;
};
