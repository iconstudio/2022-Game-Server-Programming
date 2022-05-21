#pragma once
#include "SightSector.hpp"

class SightManager
{
private:
	using mySights = std::vector<std::vector<shared_sight>>;

public:
	SightManager(float w, float h, float sector_w, float sector_h);
	~SightManager();

	void Register(const shared_ptr<GameEntity>& obj);
	void Update(const shared_ptr<GameEntity>& obj);

	const shared_sight At(int x, int y) const;
	const shared_sight At(const int_pair& coord_index) const;
	const shared_sight At(int_pair&& coord_index) const;
	shared_sight At(int x, int y);
	shared_sight At(const int_pair& coord_index);
	shared_sight At(int_pair&& coord_index);

	const shared_sight AtByPosition(float x, float y) const;
	const shared_sight AtByPosition(const XMFLOAT3& position) const;
	shared_sight AtByPosition(float x, float y);
	shared_sight AtByPosition(const XMFLOAT3& position);

	const float sizeWorldH;
	const float sizeWorldV;
	const float sizeSectorH;
	const float sizeSectorV;
	const volatile size_t countHrzSectors;
	const volatile size_t countVrtSectors;

private:
	mySights BuildSectors(size_t count_h, size_t count_v);

	int_pair ClampCoords(const XMFLOAT3& position) const;
	int_pair ClampCoords(XMFLOAT3&& position) const;

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
	mySights mySectors;
};
