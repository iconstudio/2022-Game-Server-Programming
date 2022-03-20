#include "stdafx.h"
#include "Player.h"

void Player::Render(HDC canvas)
{
	Draw::Ellipse(canvas, x - 16, y - 16, x + 16, y + 16);
}
