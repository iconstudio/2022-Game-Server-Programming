#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class SightSector
{
public:
	SightSector(int x, int y, float wx, float wy);
	~SightSector();

	const int index_x, index_y;
	const GameTransform myTransform;

	SightSector* nodeLT;
	SightSector* nodeRT;
	SightSector* nodeTP;
	SightSector* nodeBT;

private:
	shared_concurrent_vector<GameObject> seeingInstances;
	shared_concurrent_vector<GameObject>::iterator::difference_type seeingLast;
};
