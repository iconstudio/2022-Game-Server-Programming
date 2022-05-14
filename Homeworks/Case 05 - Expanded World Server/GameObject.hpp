#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void Enter(SightSector& sector);
	void Leave(SightSector& sector);

	GameTransform myTransform;

private:
	SightSector* mySector;

	const std::vector<shared_ptr<GameObject>> mySiblings;
	shared_ptr<GameObject> myChild;
};
