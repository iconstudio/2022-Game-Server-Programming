#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class SightSector
{
public:
	SightSector(int x, int y, float wx, float wy);
	~SightSector();

	bool Accept();

	bool operator==(const SightSector& other) const noexcept;

	const int index_x, index_y;
	const GameTransform myTransform;

private:
	SightSector* nodeLT;
	SightSector* nodeRT;
	SightSector* nodeTP;
	SightSector* nodeBT;

	weak_atomic_concurrent_vector<GameObject> seeingInstances;
	weak_atomic_concurrent_vector<GameObject>::iterator::difference_type seeingLast;
};
