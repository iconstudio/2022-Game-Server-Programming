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
	/// ��켱 ���: [Y][X]
	/// </summary>
	const unique_ptr<SightSector**> mySectors;

private:
	/// <summary>
	/// ������ ��ǥ�� ��ġ�� ������ ��ȣ
	/// </summary>
	/// <param name="x">���� X ��ǥ</param>
	/// <param name="y">���� Y ��ǥ</param>
	/// <returns></returns>
	constexpr int_pair PickCoords(float x, float y) const;
	/// <summary>
	/// ������ ���� �� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns></returns>
	constexpr float_pair PickPositionFirst(int x, int y) const;
	/// <summary>
	/// ������ ������ �Ʒ� ���� ��ǥ
	/// </summary>
	/// <param name="x">������ ���� ��ȣ</param>
	/// <param name="y">������ ���� ��ȣ</param>
	/// <returns></returns>
	constexpr float_pair PickPositionLast(int x, int y) const;
};