#pragma once
#include "SightSector.hpp"

class SightController
{
public:
	SightController(float width, float height, int cell_w, int cell_h);
	~SightController();

	void Update();

	SightSector& Get(int x, int y);
	SightSector& GetByPosition(float x, float y);

	const size_t countHrzSectors;
	const size_t countVrtSectors;
	const float worldHrzSize;
	const float worldVrtSize;

	const unique_ptr<SightSector**> mySectors;
};