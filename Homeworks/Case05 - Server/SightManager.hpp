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
	/// ���� ��ǥ�� ��ġ�� �þ� ������ ��ȣ
	/// </summary>
	/// <param name="x">���� X ��ǥ</param>
	/// <param name="y">���� Y ��ǥ</param>
	/// <returns>������ ��ȣ ��</returns>
	int_pair PickCoords(float x, float y) const;
	/// <param name="world_position">���� ��ǥ�� ��</param>
	/// <returns>������ ��ȣ ��</returns>
	int_pair PickCoords(float_pair world_position) const;

	/// <summary>
	/// �þ� ������ ���� �� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns>������ ���� ��ǥ</returns>
	float_pair PickPositionFirst(int x, int y) const;
	/// <param name="coord_index">������ ��ȣ ��</param>
	/// <returns>������ ���� ��ǥ</returns>
	float_pair PickPositionFirst(int_pair coord_index) const;

	/// <summary>
	/// �þ� ������ ������ �Ʒ� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns>������ ���� ��ǥ</returns>
	float_pair PickPositionLast(int x, int y) const;
	/// <param name="coord_index">������ ��ȣ ��</param>
	/// <returns>������ ���� ��ǥ</returns>
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
	/// ��켱 ���: [Y][X]
	/// </summary>
	mySights mySectors;
};
