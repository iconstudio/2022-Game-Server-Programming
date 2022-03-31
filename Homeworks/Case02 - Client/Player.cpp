#include "stdafx.h"
#include "Player.h"

void PlayerCharacter::Render(HDC canvas)
{
	auto filler = CreateSolidBrush(C_GOLD);
	auto old_filler = Draw::Attach(canvas, filler);
	auto old_align = SetTextAlign(canvas, TA_CENTER);

	const size_t sz_id = 32;
	WCHAR text_id[sz_id];
	ZeroMemory(text_id, sizeof(text_id));

	wsprintf(text_id, L"ID: %d", ID);
	TextOut(canvas, x, y, text_id, lstrlen(text_id));
	Draw::Detach(canvas, old_filler, filler);

	filler = CreateSolidBrush(C_WHITE);
	auto liner = CreatePen(0, 1, C_BLACK);
	auto old_liner = Draw::Attach(canvas, liner);

	Draw::Attach(canvas, filler);
	Draw::Ellipse(canvas, x - 16, y - 16, x + 16, y + 16);

	SetTextAlign(canvas, old_align);
	Draw::Detach(canvas, old_liner, liner);
	Draw::Detach(canvas, old_filler, filler);
}
