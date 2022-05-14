#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class SightSector
{
public:
	SightSector(int x, int y);
	~SightSector();

	const GameTransform myTransform;
	Concurrency::concurrent_vector<shared_ptr<GameObject>> seeingInstances;
};
