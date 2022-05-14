#pragma once
#include "stdafx.hpp"

class GameEntity
{
public:
	GameTransform myTransform;

	const std::vector<shared_ptr<GameEntity>> mySiblings;
	shared_ptr<GameEntity> myChild;
};
