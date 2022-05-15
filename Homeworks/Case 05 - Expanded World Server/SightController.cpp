#include "stdafx.hpp"
#include "SightController.hpp"
#include "GameObject.hpp"

SightController::SightController(float w, float h, float sector_w, float sector_h)
	: sizeWorldH(w), sizeWorldV(h)
	, sizeSectorH(sector_w), sizeSectorV(sector_h)
	, countHrzSectors(size_t(w / sector_w) + 1)
	, countVrtSectors(size_t(h / sector_h) + 1)
	, mySectors(BuildSectors(countHrzSectors, countVrtSectors))
{

}

SightController::~SightController()
{

}

void SightController::Add(const shared_ptr<GameObject>& obj)
{
	Add(*obj);
}

void SightController::Add(GameObject& obj)
{
	const auto& position = obj.GetPosition();
	auto& sector = AtByPosition(position);
	const auto& sector_before = obj.mySector.load();

	if (sector != sector_before)
	{
		if (sector_before)
		{
			obj.EnterSector(nullptr);
		}

		obj.EnterSector(sector);
	}
}

void SightController::Update(const shared_ptr<GameObject>& obj)
{
	Update(*obj);
}

void SightController::Update(GameObject& obj)
{

}

const shared_ptr<SightSector>& SightController::At(int x, int y) const
{
	return (mySectors.at(y).at(x));
}

const shared_ptr<SightSector>& SightController::At(const int_pair& coord_index) const
{
	return At(std::move(int_pair(coord_index)));
}

const shared_ptr<SightSector>& SightController::At(int_pair&& coord_index) const
{
	const auto&& coords = std::forward<int_pair>(coord_index);
	return At(coords.first, coords.second);
}

const shared_ptr<SightSector>& SightController::AtByPosition(float x, float y) const
{
	return At(PickCoords(x, y));
}

const shared_ptr<SightSector>& SightController::AtByPosition(const XMFLOAT3& position) const
{
	return At(PickCoords(position.x, position.y));
}

SightController::mySights SightController::BuildSectors(size_t count_h, size_t count_v)
{
	std::vector<std::vector<shared_ptr<SightSector>>> result(count_v);
	
	for (int i = 0; i < count_v; ++i)
	{
		std::vector<shared_ptr<SightSector>> pusher(count_h);

		for (int j = 0; j < count_h; ++j)
		{
			pusher.emplace_back(new SightSector(j, i, sizeSectorH * i, sizeSectorV * j));
		}

		result.push_back(pusher);
	}

	return result;
}

inline int_pair SightController::PickCoords(float x, float y) const
{
	const int index_x = static_cast<int>(x / sizeSectorH);
	const int index_y = static_cast<int>(y / sizeSectorV);
	return std::make_pair(index_y, index_x);
}

inline int_pair SightController::PickCoords(float_pair world_position) const
{
	return PickCoords(world_position.first, world_position.second);
}

inline float_pair SightController::PickPositionFirst(int x, int y) const
{
	const float world_x = static_cast<float>(x) * sizeSectorH;
	const float world_y = static_cast<float>(y) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

inline float_pair SightController::PickPositionFirst(int_pair coord_index) const
{
	return PickPositionFirst(coord_index.first, coord_index.second);
}

inline float_pair SightController::PickPositionLast(int x, int y) const
{
	const float world_x = static_cast<float>(x + 1) * sizeSectorH;
	const float world_y = static_cast<float>(y + 1) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

inline float_pair SightController::PickPositionLast(int_pair coord_index) const
{
	return PickPositionLast(coord_index.first, coord_index.second);
}
