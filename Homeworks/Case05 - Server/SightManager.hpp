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
	/// ���� ��ǥ�� ��ġ�� �þ� ������ ��ȣ
	/// </summary>
	/// <param name="x">���� X ��ǥ</param>
	/// <param name="y">���� Y ��ǥ</param>
	/// <returns>������ ��ȣ ��</returns>
	inline int_pair PickCoords(float x, float y) const;
	/// <param name="world_position">���� ��ǥ�� ��</param>
	/// <returns>������ ��ȣ ��</returns>
	inline int_pair PickCoords(float_pair world_position) const;

	/// <summary>
	/// �þ� ������ ���� �� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns>������ ���� ��ǥ</returns>
	inline float_pair PickPositionFirst(int x, int y) const;
	/// <param name="coord_index">������ ��ȣ ��</param>
	/// <returns>������ ���� ��ǥ</returns>
	inline float_pair PickPositionFirst(int_pair coord_index) const;

	/// <summary>
	/// �þ� ������ ������ �Ʒ� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns>������ ���� ��ǥ</returns>
	inline float_pair PickPositionLast(int x, int y) const;
	/// <param name="coord_index">������ ��ȣ ��</param>
	/// <returns>������ ���� ��ǥ</returns>
	inline float_pair PickPositionLast(int_pair coord_index) const;

	/// <summary>
	/// ��켱 ���: [Y][X]
	/// </summary>
	mySights mySectors;
};
