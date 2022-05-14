#pragma once
#include "stdafx.hpp"
#include "SightSector.hpp"

class SightController
{
private:
	using mySights = std::vector<std::vector<shared_ptr<SightSector>>>;

public:
	SightController(float w, float hw, float sector_w, float sector_h);
	~SightController();

	void Update();

	inline SightSector& At(int x, int y) const;
	inline SightSector& At(const int_pair& coord_index) const;
	inline SightSector& At(int_pair&& coord_index) const;
	inline SightSector& AtByPosition(float x, float y) const;

	const float sizeWorldH;
	const float sizeWorldV;
	const float sizeSectorH;
	const float sizeSectorV;
	const volatile size_t countHrzSectors;
	const volatile size_t countVrtSectors;

private:
	mySights BuildSectors(size_t count_h, size_t count_v);

	/// <summary>
	/// 월드 좌표에 위치한 시야 영역의 번호
	/// </summary>
	/// <param name="x">월드 X 좌표</param>
	/// <param name="y">월드 Y 좌표</param>
	/// <returns>영역의 번호 쌍</returns>
	inline int_pair PickCoords(float x, float y) const;
	/// <param name="world_position">월드 좌표의 쌍</param>
	/// <returns>영역의 번호 쌍</returns>
	inline int_pair PickCoords(float_pair world_position) const;

	/// <summary>
	/// 시야 영역의 왼쪽 위 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns>영역의 월드 좌표</returns>
	inline float_pair PickPositionFirst(int x, int y) const;
	/// <param name="coord_index">영역의 번호 쌍</param>
	/// <returns>영역의 월드 좌표</returns>
	inline float_pair PickPositionFirst(int_pair coord_index) const;

	/// <summary>
	/// 시야 영역의 오른쪽 아래 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns>영역의 월드 좌표</returns>
	inline float_pair PickPositionLast(int x, int y) const;
	/// <param name="coord_index">영역의 번호 쌍</param>
	/// <returns>영역의 월드 좌표</returns>
	inline float_pair PickPositionLast(int_pair coord_index) const;

	/// <summary>
	/// 행우선 행렬: [Y][X]
	/// </summary>
	const mySights mySectors;
};