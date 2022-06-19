#pragma once

class GameSprite {
public:
	GameSprite(HINSTANCE instance, UINT resource, UINT number = 1, int xoff = 0, int yoff = 0);
	GameSprite(LPCTSTR path, UINT number = 1, int xoff = 0, int yoff = 0);
	~GameSprite();

	void draw(HDC surface, float x, float y, float index = 0.0f, float angle = 0.0f, float xscale = 1.0f, float yscale = 1.0f, float alpha = 1.0f);

	void set_bbox(const LONG left, const LONG right, const LONG top, const LONG bottom);

	const int get_width() const;
	const int get_height() const;

	const u_int number; // 이미지 장수
	const int xoffset, yoffset; // 스프라이트의 중심점
	RECT bbox;

private:
	bool __process_image(CImage&, const size_t = 0, const size_t = 0);
	void __draw_single(HDC, CImage&, float, float, float = 0.0f, float = 1.0f, float = 1.0f, float = 1.0f);

	CImage raw; // 원본 그림.
	int raw_width, raw_height; // 원본 크기

	std::vector<unique_ptr<CImage>> frames; // 잘린 그림. 비어있을 수도 있다.
	int width, height; // 프레임 크기
};

shared_ptr<GameSprite> make_sprite(HINSTANCE instance, UINT resource, UINT number = 1, int xoff = 0, int yoff = 0);

shared_ptr<GameSprite> make_sprite(const Filepath& path, UINT number = 1, int xoff = 0, int yoff = 0);

