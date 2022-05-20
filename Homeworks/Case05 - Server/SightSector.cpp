#include "pch.hpp"
#include "stdafx.hpp"
#include "SightSector.hpp"

SightSector::SightSector(int x, int y, float w, float h)
	: index_x(x), index_y(y)
{}

SightSector::~SightSector()
{}

bool SightSector::operator==(const SightSector& other) const noexcept
{
	return other.index_x == index_x && other.index_y == index_y;
}
