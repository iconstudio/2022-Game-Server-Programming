#pragma once
#include "stdafx.h"

class SightSector
{
public:
	SightSector(int x, int y, float wx, float wy);
	~SightSector();
	


	bool operator==(const SightSector& other) const noexcept;

	const int index_x, index_y;

private:
	weak_atomic_concurrent_vector<GameObject> seeingInstances;
	weak_atomic_concurrent_vector<GameObject>::iterator::difference_type seeingLast;
};
