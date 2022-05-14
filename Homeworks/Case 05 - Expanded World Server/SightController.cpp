#include "stdafx.hpp"
#include "SightController.hpp"

SightController::SightController(float width, float height, int sector_w, int sector_h)
	: worldHrzSize(width), worldVrtSize(height)
	, worldSectorHrzSize(sector_w), worldSectorVrtSize(sector_h)
	, countHrzSectors(size_t(width / sector_w) + 1)
	, countVrtSectors(size_t(height / sector_h) + 1)
	, mySectors(nullptr)
{

}

SightController::~SightController()
{}

void SightController::Update()
{}

SightSector*& SightController::Get(int x, int y) const
{
	return (mySectors.get())[y][x];
}

SightSector*& SightController::GetByPosition(float x, float y) const
{
	const auto coords = PickCoords(x, y);
	return (mySectors.get())[coords.first][coords.second];
}

constexpr int_pair SightController::PickCoords(float x, float y) const
{
	return int_pair();
}

constexpr float_pair SightController::PickPositionFirst(int x, int y) const
{
	return float_pair();
}

constexpr float_pair SightController::PickPositionLast(int x, int y) const
{
	return float_pair();
}
