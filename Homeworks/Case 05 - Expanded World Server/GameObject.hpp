#pragma once
#include "stdafx.hpp"
#include "GameEntity.hpp"

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();


	void Enter(SightSector* sector);
	void Leave(SightSector* sector);

	GameTransform myTransform;

private:
	SightSector* mySector;

	const std::vector<shared_ptr<GameEntity>> mySiblings;
	shared_ptr<GameEntity> myChild;
};
