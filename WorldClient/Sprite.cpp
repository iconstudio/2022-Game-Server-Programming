#include "pch.hpp"
#include "Sprite.h"
#include "Draw.hpp"

GameSprite::GameSprite(HINSTANCE instance, UINT resource, UINT number, int xoff, int yoff)
	: raw(CImage()), raw_width(0), raw_height(0), width(0), height(0), number(number), xoffset(xoff), yoffset(yoff)
{
	raw.LoadFromResource(instance, resource);
	if (raw.IsNull())
	{
		WCHAR temp[256];

		swprintf_s(temp, L"자원 스프라이트 %u을(를) 불러오는 중에 오류가 발생했습니다.", resource);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"오류", MB_OK);

		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}

	raw.SetHasAlphaChannel(true);
	raw_width = raw.GetWidth();
	raw_height = raw.GetHeight();

	auto result = __process_image(raw, raw_width, raw_height);
	if (!result)
	{
		WCHAR temp[256]{};

		swprintf_s(temp, L"자원 스프라이트 %u이(가) 올바른 크기를 갖고 있지 않습니다.", resource);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"오류", MB_OK);
		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}
}

GameSprite::GameSprite(LPCTSTR path, UINT number, int xoff, int yoff)
	: raw(CImage()), raw_width(0), raw_height(0), width(0), height(0), number(number), xoffset(xoff), yoffset(yoff)
{
	raw.Load(path);
	if (raw.IsNull())
	{
		WCHAR temp[256]{};

		swprintf_s(temp, L"경로 %s에서 스프라이트를 불러올 수 없습니다.", path);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"오류", MB_OK);

		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}

	raw.SetHasAlphaChannel(true);
	raw_width = raw.GetWidth();
	raw_height = raw.GetHeight();

	auto result = __process_image(raw, raw_width, raw_height);
	if (!result)
	{
		WCHAR temp[256]{};

		swprintf_s(temp, L"%s에 위치한 그림 파일이 올바른 크기를 갖고 있지 않습니다.", path);

		int error = MessageBox(NULL, reinterpret_cast<LPCWSTR>(temp), L"오류", MB_OK);
		if (error)
		{
			SendMessage(NULL, WM_CLOSE, 0, 0);
		}
	}
}

GameSprite::~GameSprite()
{
	if (0 < number)
	{
		for (auto& image : frames)
		{
			image->Destroy();
		}
	}
	raw.Destroy();

	frames.clear();
}

void GameSprite::draw(HDC surface
	, float x, float y
	, float index
	, float angle
	, float xscale, float yscale
	, float alpha)
{
	if (1 < number)
	{
		auto frame = frames.at(static_cast<u_int>(index)).get();
		__draw_single(surface, *frame, x, y, angle, xscale, yscale, alpha);
	}
	else
	{
		__draw_single(surface, raw, x, y, angle, xscale, yscale, alpha);
	}
}

void GameSprite::set_bbox(LONG left, LONG right, LONG top, LONG bottom)
{
	bbox.left = left;
	bbox.right = right;
	bbox.top = top;
	bbox.bottom = bottom;
}

const int GameSprite::get_width() const
{
	return width;
}

const int GameSprite::get_height() const
{
	return height;
}

bool GameSprite::__process_image(CImage& image, size_t width, size_t height)
{
	if (0 < width && 0 < height)
	{
		if (1 < number)
		{ // 애니메이션을 위해서는 가로로 길쭉한 그림이 필요합니다.
			int slice_w, temp_w;
			if (1 == width)
			{
				slice_w = 1;
			}
			else
			{
				temp_w = (width / number);
				if (temp_w < 2)
				{ // 0, 1
					slice_w = 1;
				}
				else
				{
					slice_w = temp_w;
				}
			}

			// 예약
			frames.reserve(number);

			auto raw_bitlevel = raw.GetBPP();

			for (UINT i = 0; i < number; ++i)
			{
				// 프레임 삽입
				// 1. 삽입할 그림 생성
				auto image_slice = new CImage(); // make_shared<CImage>();
				image_slice->Create(slice_w, height, raw_bitlevel);
				image_slice->SetHasAlphaChannel(true);

				auto slice_buffer = image_slice->GetDC();

				// 2. 원본 그림의 (i * slice_width, 0)에 위치한 내용을 조각 그림의 (0, 0) 위치에 복사
				raw.BitBlt(slice_buffer, 0, 0, slice_w, height, i * slice_w, 0, SRCCOPY);

				// 3. 메모리 최적화
				image_slice->ReleaseDC(); // slice_buffer 해제

				// 4. 낱장 삽입 (소유권 이전으로 이제 수정 불가)
				frames.emplace_back(image_slice);
			}

			this->width = slice_w;
			this->height = raw_height;
		}
		else
		{ // one frame
			this->width = raw_width;
			this->height = raw_height;
		}

		set_bbox(-xoffset, this->width - xoffset, -yoffset, this->height - yoffset);
	}
	else
	{ // failed!
		return false;
	}

	return true;
}

void GameSprite::__draw_single(HDC surface, CImage& image
	, float dx, float dy
	, float angle
	, float xscale, float yscale
	, float alpha)
{
	int nGraphicsMode = SetGraphicsMode(surface, GM_ADVANCED);
	float cosine = static_cast<float>(lengthdir_x(1.0f, angle));
	float sine = static_cast<float>(lengthdir_y(1.0f, angle));

	// 실제와는 달리 y 좌표가 뒤집힘
	XFORM xform{};
	if (0.0 != angle)
	{
		xform.eM11 = cosine * xscale;
		xform.eM12 = sine;
		xform.eM21 = -sine;
		xform.eM22 = cosine * yscale;
	}
	else
	{
		xform.eM11 = xscale;
		xform.eM12 = 0.0f;
		xform.eM21 = 0.0f;
		xform.eM22 = yscale;
	}
	xform.eDx = floor(dx - xoffset * xscale);
	xform.eDy = floor(dy - yoffset * yscale);

	Draw::Transform(surface, xform);

	if (alpha != 1.0)
	{
		image.AlphaBlend(surface, 0, 0
			, width * abs(xscale), height * abs(yscale)
			, 0, 0
			, width, height
			, static_cast<BYTE>(255 * alpha));
	}
	else
	{
		image.Draw(surface, 0, 0
			, width * abs(xscale), height * abs(yscale)
			, 0, 0
			, width, height);
	}

	Draw::Transform(surface, Draw::transform_identity);
	SetGraphicsMode(surface, nGraphicsMode);
}

shared_ptr<GameSprite> make_sprite(HINSTANCE instance, UINT resource, UINT number, int xoff, int yoff)
{
	return make_shared<GameSprite>(instance, resource, number, xoff, yoff);
}

shared_ptr<GameSprite> make_sprite(const Filepath& path, UINT number, int xoff, int yoff)
{
	return make_shared<GameSprite>(path, number, xoff, yoff);
}
