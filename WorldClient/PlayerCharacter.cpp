#include "pch.hpp"
#include "stdafx.hpp"
#include "PlayerCharacter.hpp"
#include "Sprite.hpp"
#include "Draw.hpp"

static auto sprite = make_sprite("resource/player.png");

void PlayerCharacter::Awake()
{}

void PlayerCharacter::Start()
{
	mySprite = sprite;
}

void PlayerCharacter::Update(float time_elapsed)
{}

void PlayerCharacter::Render(HDC surface, float ax, float ay) const
{
	if (!isVisible)
		return;

	auto filler = CreateSolidBrush(C_GOLD);
	auto old_filler = Draw::Attach(surface, filler);

	auto dx = int(ax + myPosition[0]) + CELL_W / 2;
	auto dy = int(ay + myPosition[1]) + CELL_H / 2;

	mySprite->draw(surface, dx, dy);
	//Draw::Ellipse(surface, dx - 16, dy - 16, dx + 16, dy + 16);
	Draw::Detach(surface, old_filler, filler);

	filler = CreateSolidBrush(C_WHITE);
	old_filler = Draw::Attach(surface, filler);
	auto liner = CreatePen(0, 1, C_BLACK);
	auto old_liner = Draw::Attach(surface, liner);
	auto old_align = SetTextAlign(surface, TA_CENTER);

	const size_t sz_id = 32;
	WCHAR text_id[sz_id]{};
	wsprintf(text_id, L"ID: %lu", UINT(myID));

	TextOut(surface, int(dx), int(dy), text_id, lstrlen(text_id));
	SetTextAlign(surface, old_align);
	Draw::Detach(surface, old_filler, filler);
	Draw::Detach(surface, old_liner, liner);
}
