#include "pch.hpp"
#include "stdafx.hpp"
#include "PlayerCharacter.hpp"
#include "Draw.hpp"

void PlayerCharacter::Awake()
{}

void PlayerCharacter::Start()
{}

void PlayerCharacter::Update(float time_elapsed)
{}

void PlayerCharacter::Render(HDC surface, float ax, float ay) const
{
	auto filler = CreateSolidBrush(C_GOLD);
	auto old_filler = Draw::Attach(surface, filler);

	auto dx = int(ax + myPosition.x) + CELL_W * 0.5;
	auto dy = int(ax + myPosition.y) + CELL_H * 0.5;

	Draw::Ellipse(surface, dx - 16, dy - 16, dx + 16, dy + 16);
	Draw::Detach(surface, old_filler, filler);

	filler = CreateSolidBrush(C_WHITE);
	old_filler = Draw::Attach(surface, filler);
	auto liner = CreatePen(0, 1, C_BLACK);
	auto old_liner = Draw::Attach(surface, liner);
	auto old_align = SetTextAlign(surface, TA_CENTER);

	const size_t sz_id = 32;
	WCHAR text_id[sz_id];
	ZeroMemory(text_id, sizeof(text_id));
	wsprintf(text_id, L"ID: %lu", UINT(myID));

	TextOut(surface, dx, dy, text_id, lstrlen(text_id));
	SetTextAlign(surface, old_align);
	Draw::Detach(surface, old_filler, filler);
	Draw::Detach(surface, old_liner, liner);
}
