#include "pch.hpp"
#include "stdafx.hpp"
#include "SightManager.hpp"
#include "SightSector.hpp"
#include "GameObject.hpp"
#include "GameEntity.hpp"

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
{

}

void SightManager::Register(const shared_ptr<GameEntity>& obj)
{
	auto& sector = AtByPosition(obj->GetPosition());

	// 적법한 구역의 소유권 획득
	sector->Acquire();

	// 여기서 설정하지 않는다.
	//obj->SetSightArea(sector);
	sector->Add(obj->myID);

	sector->Release();
}

void SightManager::Update(const shared_ptr<GameEntity>& obj)
{
	auto& curr_sector = AtByPosition(obj->GetPosition());

	// 적법한 구역의 소유권 획득
	curr_sector->Acquire();

	// 다른 메서드라면 소유권 획득이 안된다
	auto& prev_sector = obj->GetSightArea();
	if (nullptr == prev_sector || prev_sector->TryAcquire())
	{
		if (curr_sector != prev_sector)
		{
			// NPC, 특수 개체, 플레이어의 고유 식별자
			const PID obj_id = obj->myID;

			constexpr int_pair pos_pairs[] = {
				{ -1, -1 }, { -1, 0 }, { -1, +1 },
				{  0, -1 }, {  0, 0 }, { +1, +1 },
				{ +1, -1 }, { +1, 0 }, { +1, +1 }
			};

			// 주변의 시야 구역 갱신
			for (int k = 0; k < 9; ++k)
			{
				const auto indexes = pos_pairs[k];
				const auto index_x = indexes.first;
				const auto index_y = indexes.second;

				//auto& sector = At(index_x, index_y);
			}

			// 시야 구역 내에 있는 모든 플레이어 훑기
			// 모든 플레이어의 시야 목록 갱신

			// 이전 시야 구역은 해제
			if (prev_sector)
			{
				prev_sector->Remove(obj_id);
			}
			// 시야 구역에 등록
			curr_sector->Add(obj_id);
			obj->SetSightArea(curr_sector);
		}

		// 내가 소유한 구역만 소유권 내려놓기
		prev_sector->Release();
	}

	curr_sector->Release();
}

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

const mySector& SightManager::AtByPosition(const XMFLOAT3& position) const
{
	return At(PickCoords(position.x, position.y));
}

mySector& SightManager::AtByPosition(float x, float y)
{
	return At(PickCoords(x, y));
}

mySector& SightManager::AtByPosition(const XMFLOAT3& position)
{
	return At(PickCoords(position.x, position.y));
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
