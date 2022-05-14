#pragma once
#include "SightSector.hpp"

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;

class SightController
{
public:
	SightController(float width, float height, int sector_w, int sector_h);
	~SightController();

	void Update();

	SightSector*& Get(int x, int y) const;
	SightSector*& GetByPosition(float x, float y) const;

	const float worldHrzSize;
	const float worldVrtSize;

	const float worldSectorHrzSize;
	const float worldSectorVrtSize;

	const size_t countHrzSectors;
	const size_t countVrtSectors;

	/// <summary>
	/// 행우선 행렬: [Y][X]
	/// </summary>
	const unique_ptr<SightSector**> mySectors;

private:
	/// <summary>
	/// 월드의 좌표에 위치한 영역의 번호
	/// </summary>
	/// <param name="x">월드 X 좌표</param>
	/// <param name="y">월드 Y 좌표</param>
	/// <returns></returns>
	constexpr int_pair PickCoords(float x, float y) const;
	/// <summary>
	/// 영역의 왼쪽 위 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns></returns>
	constexpr float_pair PickPositionFirst(int x, int y) const;
	/// <summary>
	/// 영역의 오른쪽 아래 월드 좌표
	/// </summary>
	/// <param name="x">영역의 가로 번호</param>
	/// <param name="y">영역의 세로 번호</param>
	/// <returns></returns>
	constexpr float_pair PickPositionLast(int x, int y) const;
};