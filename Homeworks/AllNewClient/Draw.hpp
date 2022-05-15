#pragma once

namespace Draw
{
	HGDIOBJ Attach(HDC canvas, HGDIOBJ object);
	void Detach(HDC canvas, HGDIOBJ object_old, HGDIOBJ object_new);
	void Clear(HDC canvas, int width, int height, COLORREF color);
	BOOL SizedRect(HDC canvas, int x, int y, int w, int h);
	BOOL Rect(HDC canvas, int x1, int y1, int x2, int y2);
	BOOL Ellipse(HDC canvas, int x1, int y1, int x2, int y2);

	void Transform(HDC, XFORM&);
	void ResetTransform(HDC);

	static const XFORM transform_identity;
}

