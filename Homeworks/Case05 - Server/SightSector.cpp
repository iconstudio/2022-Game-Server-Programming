#include "pch.hpp"
#include "stdafx.hpp"
#include "SightSector.hpp"
#include "GameEntity.hpp"

SightSector::SightSector(int x, int y, float w, float h)
	: index_x(x), index_y(y)
	, isOwned()
	, seeingInstances(), seeingLast()
{
	seeingInstances.reserve(10);
}

SightSector::~SightSector()
{}

void SightSector::Acquire()
{
	isOwned.test_and_set(std::memory_order_acquire);
}

void SightSector::Release()
{
	isOwned.clear(std::memory_order_release);
}

bool SightSector::TryAcquire()
{
	return !isOwned.test_and_set(std::memory_order_acquire);
}

void SightSector::Add(const PID id)
{
	mySight.insert(id);
}

void SightSector::Remove(const PID id)
{
	if (auto it = mySight.find(id); mySight.end() != it)
	{
		mySight.unsafe_erase(it);
	}
}

std::vector<PID> SightSector::GetSightList() const
{
	return std::vector<PID>{ mySight.begin(), mySight.end() };
}

bool SightSector::operator==(const SightSector& other) const noexcept
{
	return other.index_x == index_x && other.index_y == index_y;
}
