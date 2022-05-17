#include "stdafx.h"
#include "GameEntity.hpp"

GameEntity::GameEntity(PID id, int x, int y)
	: myID(id), x(x), y(y)
{}

GameEntity::~GameEntity()
{}

