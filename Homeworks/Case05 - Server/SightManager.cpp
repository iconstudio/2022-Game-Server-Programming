#include "pch.hpp"
#include "stdafx.hpp"
#include "SightManager.hpp"
#include "SightSector.hpp"
#include "GameObject.hpp"
#include "GameEntity.hpp"

SightManager::SightManager(float w, float h, float sector_w, float sector_h)
	: sizeWorldH(w), sizeWorldV(h)
	, sizeSectorH(sector_w), sizeSectorV(sector_h)
	, countHrzSectors(size_t(std::ceil(w / sector_w)))
	, countVrtSectors(size_t(std::ceil(h / sector_h)))
	, mySectors(BuildSectors(countHrzSectors, countVrtSectors))
{

}

SightManager::~SightManager()
{

}

void SightManager::Register(const shared_ptr<GameEntity>& obj)
{
	AtByPosition(obj->GetPosition()).load()->Add(obj);
}

void SightManager::Update(const shared_ptr<GameEntity>& obj)
{
	auto curr_sector = AtByPosition(obj->GetPosition());
	auto curr = curr_sector.load(std::memory_order_acquire);

	auto& prev_sector = obj->mySightSector;
	auto prev = prev_sector.load(std::memory_order_acquire);

	if (prev != curr)
	{
		prev->Remove(obj);
		curr->Add(obj);
		prev_sector.store(curr, std::memory_order_release);
	}
	else
	{
		prev_sector.store(prev, std::memory_order_release);
	}

	curr_sector.store(curr, std::memory_order_release);
}

const shared_sight SightManager::At(int x, int y) const
{
	return (mySectors.at(y).at(x).load(std::memory_order_relaxed));
}

const shared_sight SightManager::At(const int_pair& coord_index) const
{
	return At(std::move(int_pair(coord_index)));
}

const shared_sight SightManager::At(int_pair&& coord_index) const
{
	const auto&& coords = std::forward<int_pair>(coord_index);
	return At(coords.first, coords.second);
}

shared_sight SightManager::At(int x, int y)
{
	return (mySectors.at(y).at(x).load(std::memory_order_relaxed));
}

shared_sight SightManager::At(const int_pair& coord_index)
{
	return At(std::move(int_pair(coord_index)));
}

shared_sight SightManager::At(int_pair&& coord_index)
{
	const auto&& coords = std::forward<int_pair>(coord_index);
	return At(coords.first, coords.second);
}

const shared_sight SightManager::AtByPosition(float x, float y) const
{
	return At(PickCoords(x, y));
}

const shared_sight SightManager::AtByPosition(const XMFLOAT3& position) const
{
	return At(PickCoords(position.x, position.y));
}

shared_sight SightManager::AtByPosition(float x, float y)
{
	return At(PickCoords(x, y));
}

shared_sight SightManager::AtByPosition(const XMFLOAT3& position)
{
	return At(PickCoords(position.x, position.y));
}

SightManager::mySights SightManager::BuildSectors(size_t count_h, size_t count_v)
{
	std::vector<std::vector<shared_sight>> result(count_v);

	for (int i = 0; i < count_v; ++i)
	{
		std::vector<shared_sight> pusher(count_h);

		for (int j = 0; j < count_h; ++j)
		{
			auto ptr = make_shared<SightSector>(j, i, sizeSectorH * i, sizeSectorV * j);
			
			//shared_sight sight{};
			//sight.store(ptr, std::memory_order_relaxed);

			//pusher.push_back(sight);
		}

		result.push_back(pusher);
	}

	return result;
}

int_pair SightManager::ClampCoords(const XMFLOAT3& position) const
{
	return ClampCoords(std::move(XMFLOAT3(position)));
}

int_pair SightManager::ClampCoords(XMFLOAT3&& position) const
{
	const auto&& pos = std::forward<XMFLOAT3>(position);
	const float clamped_x = std::clamp(0.0f, pos.x, sizeWorldH);
	const float clamped_y = std::clamp(0.0f, pos.y, sizeWorldV);

	return PickCoords(clamped_x, clamped_y);
}

inline int_pair SightManager::PickCoords(float x, float y) const
{
	const int index_x = static_cast<int>(x / sizeSectorH);
	const int index_y = static_cast<int>(y / sizeSectorV);

	return std::make_pair(index_y, index_x);
}

inline int_pair SightManager::PickCoords(float_pair world_position) const
{
	return PickCoords(world_position.first, world_position.second);
}

inline float_pair SightManager::PickPositionFirst(int x, int y) const
{
	const float world_x = static_cast<float>(x) * sizeSectorH;
	const float world_y = static_cast<float>(y) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

inline float_pair SightManager::PickPositionFirst(int_pair coord_index) const
{
	return PickPositionFirst(coord_index.first, coord_index.second);
}

inline float_pair SightManager::PickPositionLast(int x, int y) const
{
	const float world_x = static_cast<float>(x + 1) * sizeSectorH;
	const float world_y = static_cast<float>(y + 1) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

inline float_pair SightManager::PickPositionLast(int_pair coord_index) const
{
	return PickPositionLast(coord_index.first, coord_index.second);
}
