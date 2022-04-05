#pragma once
#include "stdafx.h"

class PlayerCharacter
{
public:
	void Render(HDC canvas);

	PID ID;
	UCHAR x, y;
};
