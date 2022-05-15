#include "stdafx.hpp"
#include "SightSector.hpp"

SightSector::SightSector(int x, int y, float wx, float wy)
	: index_x(x), index_y(y)
	, myTransform(wx, wy, 0.0f)
	, nodeLT(nullptr), nodeRT(nullptr), nodeTP(nullptr), nodeBT(nullptr)
	, seeingInstances(100), seeingLast()
{}

SightSector::~SightSector()
{}
