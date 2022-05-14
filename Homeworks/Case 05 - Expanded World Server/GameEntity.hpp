#pragma once
#include "stdafx.hpp"

class GameEntity
{
public:
	const std::vector<shared_ptr<GameEntity>> mySiblings;
	shared_ptr<GameEntity> myChild;
};
