#include "pch.hpp"
#include "SightSector.hpp"

SightSector::SightSector(int x, int y, float w, float h)
	: index_x(x), index_y(y)
	, isOwned()
	, mySight()
{
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
	auto it = mySight.find(id);
	if (mySight.end() != it)
	{
		mySight.erase(it);
	}
}

std::unordered_set<PID> SightSector::GetSightList() const
{
	return mySight;
}

bool SightSector::operator==(const SightSector& other) const noexcept
{
	return other.index_x == index_x && other.index_y == index_y;
}
