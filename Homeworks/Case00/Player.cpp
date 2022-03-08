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
{}

void Player::MoveRT()
{}

void Player::MoveUP()
{}

void Player::MoveDW()
{}
