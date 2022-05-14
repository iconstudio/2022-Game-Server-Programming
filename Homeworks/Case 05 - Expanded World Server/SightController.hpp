#pragma once
#include "SightSector.hpp"

class SightController
{
public:
	SightController(int width, int height);

	unique_ptr<SightSector**> mySectors;
};