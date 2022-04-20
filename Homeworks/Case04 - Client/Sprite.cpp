#include "stdafx.h"
#include "Sprite.hpp"

Sprite::Sprite(const Path& path, UINT number, int xoff, int yoff)
	: sprNumber(number), sprOffsetX(xoff), sprOffsetY(yoff)
	, sprFrames(number), sprWidth(0), sprHeight(0)
	, rawImage(std::make_unique<CImage>()), rawWidth(0), rawHeight(0)
{
	auto directory = path.c_str();
	rawImage->Load(directory);
	if (rawImage->IsNull())
	{
		WCHAR temp[256];
		swprintf_s(temp, L"��� %s���� ��������Ʈ�� �ҷ��� �� �����ϴ�.", directory);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"����", MB_OK);

		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}

	rawWidth = rawImage->GetWidth();
	rawHeight = rawImage->GetHeight();
	if (0 < rawWidth && 0 < rawHeight)
	{
		rawImage->SetHasAlphaChannel(true);

		if (1 < number) // �ִϸ��̼��� ���ؼ��� ���η� ������ �׸��� �ʿ��մϴ�.
		{
			UINT slice_w, temp_w;
			if (1 == rawWidth)
			{
				slice_w = 1;
			}
			else
			{
				temp_w = (rawWidth / number);
				if (temp_w < 2) // 0, 1
				{
					slice_w = 1;
				}
				else
				{
					slice_w = temp_w;
				}
			}

			auto raw_bitlevel = rawImage->GetBPP();

			// ������ ����
			for (UINT i = 0; i < number; ++i)
			{ 
				// 1. ������ �׸� ����
				auto image_slice = std::make_unique<CImage>();
				image_slice->Create(slice_w, rawHeight, raw_bitlevel);
				image_slice->SetHasAlphaChannel(true);

				auto slice_buffer = image_slice->GetDC();

				// 2. ���� �׸��� (i * slice_width, 0)�� ��ġ�� ������ ���� �׸��� (0, 0) ��ġ�� ����
				rawImage->BitBlt(slice_buffer, 0, 0, slice_w, rawHeight, i * slice_w, 0, SRCCOPY);

				// 3. �޸� ����ȭ
				image_slice->ReleaseDC(); // slice_buffer ����

				// 4. ���� ���� (������ �������� ���� ���� �Ұ�)
				sprFrames.emplace_back(std::move(image_slice));
			}

			sprWidth = slice_w;
			sprHeight = rawHeight;
		}
		else
		{
			// single frame
			sprWidth = rawWidth;
			sprHeight = rawHeight;
		}
	}
	else
	{
		WCHAR temp[256];
		swprintf_s(temp, L"%s�� ��ġ�� �׸� ������ �ùٸ� ũ�⸦ ���� ���� �ʽ��ϴ�.", directory);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"����", MB_OK);
		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}
}

Sprite::~Sprite()
{}

constexpr UINT Sprite::GetLength() const
{
	return sprNumber;
}

constexpr UINT Sprite::GetWidth() const
{
	return sprWidth;
}

constexpr UINT Sprite::GetHeight() const
{
	return sprHeight;
}

constexpr UINT Sprite::GetXOffset() const
{
	return sprOffsetX;
}

constexpr UINT Sprite::GetYOffset() const
{
	return sprOffsetY;
}

void Sprite::Draw(HDC surface, float x, float y, float ind, float angle, float xs, float ys, float alpha)
{
	if (1 < sprNumber)
	{
		auto& frame = sprFrames.at(static_cast<UINT>(ind) % sprNumber);
		DrawSingle(surface, *frame, x, y, angle, xs, ys, alpha);
	}
	else
	{
		DrawSingle(surface, *rawImage, x, y, angle, xs, ys, alpha);
	}
}

void Sprite::DrawSingle(HDC surface, CImage& image, float x, float y, float angle, float xs, float ys, float alpha)
{
	if (0.0 != angle)
	{
		auto cosine = (float)dcos(angle);
		auto sine = (float)dsin(angle);

		int center_x = x, center_y = y;

		int nGraphicsMode = SetGraphicsMode(surface, GM_ADVANCED);

		// �����ʹ� �޸� y ��ǥ�� ������
		XFORM xform{};
		xform.eM11 = cosine;
		xform.eM12 = sine;
		xform.eM21 = -sine;
		xform.eM22 = cosine;
		xform.eDx = (center_x - cosine * center_x + sine * center_y);
		xform.eDy = (center_y - cosine * center_y - sine * center_x);

		Draw::Transform(surface, xform);

		center_x -= sprOffsetX * xs;
		center_y -= sprOffsetY * ys;

		auto width = sprWidth * std::abs(xs);
		auto height = sprHeight * std::abs(ys);
		if (1.0f != alpha)
		{
			auto opacity = BYTE(255.0f * alpha);
			image.AlphaBlend(surface, center_x, center_y, width, height, 0, 0, sprWidth, sprHeight, opacity);
		}
		else
		{
			image.Draw(surface, center_x, center_y, width, height, 0, 0, sprWidth, sprHeight);
		}

		Draw::ResetTransform(surface);
		SetGraphicsMode(surface, nGraphicsMode);
	}
	else
	{
		int tx = (int)(x - sprOffsetX * xs);
		int ty = (int)(y - sprOffsetY * ys);
		auto width = sprWidth * std::abs(xs);
		auto height = sprHeight * std::abs(ys);

		if (1.0f != alpha)
		{
			auto opacity = BYTE(255.0f * alpha);
			image.AlphaBlend(surface, tx, ty, width, height, 0, 0, sprWidth, sprHeight, opacity);
		}
		else
		{
			image.Draw(surface, tx, ty, width, height, 0, 0, sprWidth, sprHeight);
		}
	}
}
