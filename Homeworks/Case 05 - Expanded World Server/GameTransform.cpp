#include "stdafx.hpp"
#include "GameTransform.hpp"

GameTransform::GameTransform()
	: GameTransform(0.0f, 0.0f, 0.0f)
{}

GameTransform::GameTransform(float x, float y, float z)
	: x(x), y(y), z(z)
{}
