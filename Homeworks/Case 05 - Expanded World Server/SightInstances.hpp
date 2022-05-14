#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class SightInstances
{
public:
	SightInstances(SightSector& sector, float x, float y);


	SightSector& mySector;

	GameTransform myTransform;
	Concurrency::concurrent_vector<shared_ptr<GameObject>> seeingInstances;
};
