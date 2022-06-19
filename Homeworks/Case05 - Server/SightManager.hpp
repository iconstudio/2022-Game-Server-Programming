#pragma once
#include "SightSector.hpp"

using mySector = shared_ptr<SightSector>;

class SightManager
{
private:
	using mySights = std::vector<std::vector<mySector>>;

public:
	SightManager(IOCPFramework& framework, float w, float h, float sector_w, float sector_h);
	~SightManager();

	const mySector& At(int x, int y) const;
	const mySector& At(const int_pair& coord_index) const;
	const mySector& At(int_pair&& coord_index) const;
	mySector& At(int x, int y);
	mySector& At(const int_pair& coord_index);
	mySector& At(int_pair&& coord_index);

	const mySector& AtByPosition(float x, float y) const;
	const mySector& AtByPosition(const float_pair& pos) const;
	const mySector& AtByPosition(const float (&position)[2]) const;
	mySector& AtByPosition(float x, float y);
	mySector& AtByPosition(const float (&position)[2]);

	/// <summary>
	/// 월드 좌표에 위치한 시야 영역의 번호
	/// </summary>
	/// <param name="x">월드 X 좌표</param>
	/// <param name="y">월드 Y 좌표</param>
	/// <returns>영역의 번호 쌍</returns>
	int_pair PickCoords(float x, float y) const;
	/// <param name="world_position">월드 좌표의 쌍</param>
	/// <returns>영역의 번호 쌍</returns>
	int_pair PickCoords(float_pair world_position) const;

	/// <summary>
	/// 시야 영역의 왼쪽 위 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns>영역의 월드 좌표</returns>
	float_pair PickPositionFirst(int x, int y) const;
	/// <param name="coord_index">영역의 번호 쌍</param>
	/// <returns>영역의 월드 좌표</returns>
	float_pair PickPositionFirst(int_pair coord_index) const;

	/// <summary>
	/// 시야 영역의 오른쪽 아래 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns>영역의 월드 좌표</returns>
	float_pair PickPositionLast(int x, int y) const;
	/// <param name="coord_index">영역의 번호 쌍</param>
	/// <returns>영역의 월드 좌표</returns>
	float_pair PickPositionLast(int_pair coord_index) const;

	const float sizeWorldH;
	const float sizeWorldV;
	const float sizeSectorH;
	const float sizeSectorV;
	const volatile size_t countHrzSectors;
	const volatile size_t countVrtSectors;

private:
	mySights BuildSectors(size_t count_h, size_t count_v);

	int_pair ClampCoords(const float_pair& position) const;
	int_pair ClampCoords(const float(&position)[2]) const;

	IOCPFramework& myFramework;

	/// <summary>
	/// 행우선 행렬: [Y][X]
	/// </summary>
	mySights mySectors;
};
