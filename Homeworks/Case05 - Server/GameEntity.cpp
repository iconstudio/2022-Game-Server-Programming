#include "stdafx.hpp"
#include "GameEntity.hpp"

GameEntity::GameEntity(PID id, float x, float y, float z)
	: GameEntity(id, XMFLOAT3(x, y, z))
{}

GameEntity::GameEntity(PID id, const XMFLOAT3& pos)
	: GameEntity(id, XMFLOAT3(pos))
{}

GameEntity::GameEntity(PID id, XMFLOAT3&& pos)
	: GameObject(std::forward<XMFLOAT3>(pos))
	, myID(id)
{}

GameEntity::~GameEntity()
{}

