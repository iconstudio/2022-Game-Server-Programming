#pragma once
#include "stdafx.hpp"

using Path = std::filesystem::path;

class Sprite
{
public:
	Sprite(const Path& path, UINT number = 1, int xoff = 0, int yoff = 0);
	~Sprite();

	constexpr UINT GetLength() const;
	constexpr UINT GetWidth() const;
	constexpr UINT GetHeight() const;
	constexpr UINT GetXOffset() const;
	constexpr UINT GetYOffset() const;

	void Draw(HDC surface, float x, float y, float ind = 0.0, float angle = 0.0, float xs = 1.0, float ys = 1.0, float alpha = 1.0);

	const u_int sprNumber; // �̹��� ���
	const int sprOffsetX, sprOffsetY; // ��������Ʈ�� �߽���

private:
	void DrawSingle(HDC surface, CImage& image, float x, float y, float angle, float xs, float ys, float alpha);

	std::unique_ptr<CImage> rawImage; // ���� �׸�.
	UINT rawWidth, rawHeight; // ���� ũ��

	std::vector<std::unique_ptr<CImage>> sprFrames; // �߸� �׸�. ������� ���� �ִ�.
	UINT sprWidth, sprHeight; // ������ ũ��
};
