#include "pch.hpp"
#include "stdafx.hpp"
#include "GameEntity.hpp"
#include "Sprite.hpp"
#include "Draw.hpp"

static auto sprite_mob_skel = make_sprite("resource/enemy_skeleton.png");
static auto sprite_mob_skelking = make_sprite("resource/enemy_skelking.png");
static auto sprite_mob_demon = make_sprite("resource/enemy_demon.png");

GameEntity::GameEntity()
	: GameObject()
	, myID(0)
{}

GameEntity::~GameEntity()
{}

void GameEntity::Awake()
{}

void GameEntity::Start()
{
	if (myCategory == ENTITY_CATEGORY::MOB)
	{
		switch (myType)
		{
			case ENTITY_TYPES::MOB_DEMON:
			{
				mySprite = sprite_mob_demon;
			}
			break;

			case ENTITY_TYPES::MOB_SKELETON:
			{
				mySprite = sprite_mob_skel;
			}
			break;

			case ENTITY_TYPES::MOB_SKELETON_KING:
			{
				mySprite = sprite_mob_skelking;
			}
			break;
		}
	}
}

void GameEntity::Update(float time_elapsed)
{}

void GameEntity::Render(HDC surface) const
{
	GameEntity::Render(surface, 0.0f, 0.0f);
}

void GameEntity::Render(HDC surface, float ax, float ay) const
{
	if (!isVisible)
		return;

	auto dx = int(ax + myPosition[0]) + CELL_W / 2;
	auto dy = int(ay + myPosition[1]) + CELL_H / 2;

	if (mySprite)
	{
		mySprite->draw(surface, dx, dy);
	}

	auto filler = CreateSolidBrush(C_WHITE);
	auto old_filler = Draw::Attach(surface, filler);
	auto liner = CreatePen(0, 1, C_BLACK);
	auto old_liner = Draw::Attach(surface, liner);
	auto old_align = SetTextAlign(surface, TA_CENTER);

	const size_t sz_id = 32;
	WCHAR text_id[sz_id]{};
	wsprintf(text_id, L"ID: %lu", UINT(myID));

	TextOut(surface, int(dx), int(dy) + 16, text_id, lstrlen(text_id));
	SetTextAlign(surface, old_align);
	Draw::Detach(surface, old_filler, filler);
	Draw::Detach(surface, old_liner, liner);
}
