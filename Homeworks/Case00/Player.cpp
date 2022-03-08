#include "stdafx.h"
#include "Player.h"

void Player::Update(float delta_time)
{
	auto check_lt = GetAsyncKeyState(VK_LEFT);
	auto check_up = GetAsyncKeyState(VK_UP);
	auto check_rt = GetAsyncKeyState(VK_RIGHT);
	auto check_dw = GetAsyncKeyState(VK_DOWN);


}

void Player::Render(HDC canvas)
{
	Draw::Rect(canvas, x, y, x + 50, y + 50);
}

void Player::MoveLT()
{
	x -= 32;
}

void Player::MoveRT()
{
	x += 32;
}

void Player::MoveUP()
{
	y -= 32;
}

void Player::MoveDW()
{
	y += 32;
}
