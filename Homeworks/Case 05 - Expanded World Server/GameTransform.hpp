#pragma once
#include "stdafx.hpp"

class GameTransform
{
public:
	GameTransform();
	GameTransform(float x, float y, float z);

	XMFLOAT3 myPosition;
};
