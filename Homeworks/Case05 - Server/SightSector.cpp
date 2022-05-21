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

void SightSector::Add(const shared_ptr<GameEntity>& entity)
{
	Add(std::move(shared_ptr<GameEntity>(entity)));
}

void SightSector::Add(shared_ptr<GameEntity>&& entity)
{
	seeingInstances.emplace_back(std::forward<shared_ptr<GameEntity>>(entity));

	// 플레이어 캐릭터는 앞으로 당기기
	seeingLast = std::partition(seeingInstances.begin()
		, seeingInstances.end()
		, [](const shared_ptr<GameEntity>& inst) {
		return CLIENTS_ORDER_BEGIN <= inst->myID;
	});
}

void SightSector::Remove(const shared_ptr<GameEntity>& entity)
{}

void SightSector::Remove(shared_ptr<GameEntity>&& entity)
{}

bool SightSector::operator==(const SightSector& other) const noexcept
{
	return other.index_x == index_x && other.index_y == index_y;
}
