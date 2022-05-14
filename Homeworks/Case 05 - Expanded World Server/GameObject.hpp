#pragma once
#include "stdafx.hpp"
#include "GameEntity.hpp"

class GameObject : public GameEntity
{
public:
	GameObject();
	virtual ~GameObject();

	void Enter(SightSector& sector);
	void Leave(SightSector& sector);

	SightSector* mySector;
};
