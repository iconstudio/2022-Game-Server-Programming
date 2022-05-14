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
	const mySights mySectors;
};