#include "stdafx.hpp"
#include "SightSector.hpp"

SightSector::SightSector(int x, int y)
	: myTransform(x, y, 0.0f)
	, seeingInstances(100)
{}

SightSector::~SightSector()
{}
