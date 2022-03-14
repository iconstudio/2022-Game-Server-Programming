#include "stdafx.h"
#include "Player.h"
#include "Framework.h"

void Player::Update(float delta_time)
{
}

void Player::Render(HDC canvas)
{
	Draw::Ellipse(canvas, x - 16, y - 16, x + 16, y + 16);
}
