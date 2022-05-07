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

	const u_int sprNumber; // 이미지 장수
	const int sprOffsetX, sprOffsetY; // 스프라이트의 중심점

private:
	void DrawSingle(HDC surface, CImage& image, float x, float y, float angle, float xs, float ys, float alpha);

	std::unique_ptr<CImage> rawImage; // 원본 그림.
	UINT rawWidth, rawHeight; // 원본 크기

	std::vector<std::unique_ptr<CImage>> sprFrames; // 잘린 그림. 비어있을 수도 있다.
	UINT sprWidth, sprHeight; // 프레임 크기
};
