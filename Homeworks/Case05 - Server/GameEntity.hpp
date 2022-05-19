#pragma once
#include "GameObject.hpp"

class GameEntity : public GameObject
{
public:
	GameEntity(PID id, float x, float y, float z);
	GameEntity(PID id, const XMFLOAT3& pos);
	GameEntity(PID id, XMFLOAT3&& pos);
	virtual ~GameEntity();

	PID myID;
};
