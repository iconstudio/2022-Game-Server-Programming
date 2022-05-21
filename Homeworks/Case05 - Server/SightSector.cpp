#include "pch.hpp"
#include "stdafx.hpp"
#include "SightSector.hpp"

SightSector::SightSector(int x, int y, float w, float h)
	: index_x(x), index_y(y)
{}

SightSector::~SightSector()
{}

void SightSector::Add(const shared_ptr<GameEntity>& entity)
{
	Add(std::move(shared_ptr<GameEntity>(entity)));
}

void SightSector::Add(shared_ptr<GameEntity>&& entity)
{
	seeingInstances.emplace_back(std::forward<shared_ptr<GameEntity>>(entity));
}

void SightSector::Remove(const shared_ptr<GameEntity>& entity)
{}

void SightSector::Remove(shared_ptr<GameEntity>&& entity)
{}

bool SightSector::operator==(const SightSector& other) const noexcept
{
	return other.index_x == index_x && other.index_y == index_y;
}
