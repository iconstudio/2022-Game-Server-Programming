#include "pch.hpp"
#include "SightManager.hpp"
#include "SightSector.hpp"
#include "GameObject.hpp"
#include "Framework.hpp"

SightManager::SightManager(IOCPFramework& framework, float w, float h, float sector_w, float sector_h)
	: myFramework(framework)
	, sizeWorldH(w), sizeWorldV(h)
	, sizeSectorH(sector_w), sizeSectorV(sector_h)
	, countHrzSectors(size_t(std::ceil(w / sector_w)))
	, countVrtSectors(size_t(std::ceil(h / sector_h)))
	, mySectors(BuildSectors(countHrzSectors, countVrtSectors))
{

}

SightManager::~SightManager()
{}

const mySector& SightManager::At(int x, int y) const
{
	return (mySectors.at(y).at(x));
}

const mySector& SightManager::At(const int_pair& coord_index) const
{
	return At(std::move(int_pair(coord_index)));
}

const mySector& SightManager::At(int_pair&& coord_index) const
{
	const auto&& coords = std::forward<int_pair>(coord_index);
	return At(coords.first, coords.second);
}

mySector& SightManager::At(int x, int y)
{
	return (mySectors.at(y).at(x));
}

mySector& SightManager::At(const int_pair& coord_index)
{
	return At(std::move(int_pair(coord_index)));
}

mySector& SightManager::At(int_pair&& coord_index)
{
	const auto&& coords = std::forward<int_pair>(coord_index);
	return At(coords.first, coords.second);
}

const mySector& SightManager::AtByPosition(float x, float y) const
{
	return At(PickCoords(x, y));
}

const mySector& SightManager::AtByPosition(const float_pair& pos) const
{
	return At(PickCoords(pos.first, pos.second));
}

const mySector& SightManager::AtByPosition(const float(&position)[2]) const
{
	return At(PickCoords(position[0], position[1]));
}

mySector& SightManager::AtByPosition(float x, float y)
{
	return At(PickCoords(x, y));
}

mySector& SightManager::AtByPosition(const float(&position)[2])
{
	return At(PickCoords(position[0], position[1]));
}

SightManager::mySights SightManager::BuildSectors(size_t count_h, size_t count_v)
{
	std::vector<std::vector<mySector>> result{};
	result.reserve(count_v);

	for (int i = 0; i < count_v; ++i)
	{
		std::vector<mySector> pusher{};
		pusher.reserve(count_h);

		for (int j = 0; j < count_h; ++j)
		{
			auto ptr = make_shared<SightSector>(j, i, sizeSectorH * i, sizeSectorV * j);

			pusher.push_back(ptr);
		}

		//result.assign(pusher.begin(), pusher.end());
		result.push_back(pusher);
	}

	return result;
}

int_pair SightManager::ClampCoords(const float_pair& position) const
{
	return ClampCoords({ position.first, position.second });
}

int_pair SightManager::ClampCoords(const float(&position)[2]) const
{
	const float clamped_x = std::clamp(0.0f, position[0], sizeWorldH);
	const float clamped_y = std::clamp(0.0f, position[1], sizeWorldV);

	return PickCoords(clamped_x, clamped_y);
}

int_pair SightManager::PickCoords(float x, float y) const
{
	const int index_x = std::clamp(static_cast<int>(x / sizeSectorH), 0, WORLD_CELLS_CNT_H - 1);
	const int index_y = std::clamp(static_cast<int>(y / sizeSectorV), 0, WORLD_CELLS_CNT_V - 1);

	return std::make_pair(index_y, index_x);
}

int_pair SightManager::PickCoords(float_pair world_position) const
{
	return PickCoords(world_position.first, world_position.second);
}

float_pair SightManager::PickPositionFirst(int x, int y) const
{
	const float world_x = static_cast<float>(x) * sizeSectorH;
	const float world_y = static_cast<float>(y) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

float_pair SightManager::PickPositionFirst(int_pair coord_index) const
{
	return PickPositionFirst(coord_index.first, coord_index.second);
}

float_pair SightManager::PickPositionLast(int x, int y) const
{
	const float world_x = static_cast<float>(x + 1) * sizeSectorH;
	const float world_y = static_cast<float>(y + 1) * sizeSectorV;
	return std::make_pair(world_y, world_x);
}

float_pair SightManager::PickPositionLast(int_pair coord_index) const
{
	return PickPositionLast(coord_index.first, coord_index.second);
}
