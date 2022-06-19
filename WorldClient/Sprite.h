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

	const u_int number; // �̹��� ���
	const int xoffset, yoffset; // ��������Ʈ�� �߽���
	RECT bbox;

private:
	bool __process_image(CImage&, const size_t = 0, const size_t = 0);
	void __draw_single(HDC, CImage&, float, float, float = 0.0f, float = 1.0f, float = 1.0f, float = 1.0f);

	CImage raw; // ���� �׸�.
	int raw_width, raw_height; // ���� ũ��

	std::vector<unique_ptr<CImage>> frames; // �߸� �׸�. ������� ���� �ִ�.
	int width, height; // ������ ũ��
};

shared_ptr<GameSprite> make_sprite(HINSTANCE instance, UINT resource, UINT number = 1, int xoff = 0, int yoff = 0);

shared_ptr<GameSprite> make_sprite(const Filepath& path, UINT number = 1, int xoff = 0, int yoff = 0);

