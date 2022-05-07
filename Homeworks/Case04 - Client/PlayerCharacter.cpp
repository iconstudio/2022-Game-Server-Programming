#include "stdafx.hpp"
#include "PlayerCharacter.hpp"
#include "Network.hpp"

void PlayerCharacter::Render(HDC canvas)
{
	auto filler = CreateSolidBrush(C_GOLD);
	auto old_filler = Draw::Attach(canvas, filler);

	auto dx = BOARD_X + x * CELL_W + CELL_W * 0.5;
	auto dy = BOARD_Y + y * CELL_H + CELL_W * 0.5;

	Draw::Ellipse(canvas, dx - 16, dy - 16, dx + 16, dy + 16);
	Draw::Detach(canvas, old_filler, filler);

	filler = CreateSolidBrush(C_WHITE);
	old_filler = Draw::Attach(canvas, filler);
	auto liner = CreatePen(0, 1, C_BLACK);
	auto old_liner = Draw::Attach(canvas, liner);
	auto old_align = SetTextAlign(canvas, TA_CENTER);

	const size_t sz_id = 32;
	WCHAR text_id[sz_id];
	ZeroMemory(text_id, sizeof(text_id));
	wsprintf(text_id, L"ID: %u", ID);

	TextOut(canvas, dx, dy, text_id, lstrlen(text_id));
	SetTextAlign(canvas, old_align);
	Draw::Detach(canvas, old_filler, filler);
	Draw::Detach(canvas, old_liner, liner);
}
