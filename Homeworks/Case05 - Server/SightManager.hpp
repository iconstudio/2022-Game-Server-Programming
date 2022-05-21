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

	/// <summary>
	/// 객체를 시야 관리기에 등록한다.
	/// 그러나 이것만으로는 시야를 갱신하지 않는다.
	/// </summary>
	/// <param name="entity">등록할 개체 (NPC, 특수 개체, 플레이어)</param>
	void Register(const shared_ptr<GameEntity>& entity);
	/// <summary>
	/// 객체 근처에 있는 세션의 시야 목록을 갱신한다.
	/// 상, 하, 좌, 우, 대각선 9개의 목록을 전송한다.
	/// 이때 변화한 점만 전송한다. 변화하는 즉시 전송한다.
	/// </summary>
	/// <param name="entity">갱신할 개체 (NPC, 특수 개체, 플레이어)</param>
	void Update(const shared_ptr<GameEntity>& entity);

	const mySector& At(int x, int y) const;
	const mySector& At(const int_pair& coord_index) const;
	const mySector& At(int_pair&& coord_index) const;
	mySector& At(int x, int y);
	mySector& At(const int_pair& coord_index);
	mySector& At(int_pair&& coord_index);

	const mySector& AtByPosition(float x, float y) const;
	const mySector& AtByPosition(const XMFLOAT3& position) const;
	mySector& AtByPosition(float x, float y);
	mySector& AtByPosition(const XMFLOAT3& position);

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

	IOCPFramework& myFramework;

	/// <summary>
	/// 행우선 행렬: [Y][X]
	/// </summary>
	mySights mySectors;
};
