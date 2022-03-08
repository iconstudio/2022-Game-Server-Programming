#include "stdafx.h"
#include "Player.h"

void Player::Update(float delta_time)
{
	auto val_lt = GetAsyncKeyState(VK_LEFT);
	auto val_up = GetAsyncKeyState(VK_UP);
	auto val_rt = GetAsyncKeyState(VK_RIGHT);
	auto val_dw = GetAsyncKeyState(VK_DOWN);

	bool check_lt = 0x8000 & val_lt;
	bool check_up = 0x8000 & val_up;
	bool check_rt = 0x8000 & val_rt;
	bool check_dw = 0x8000 & val_dw;

	if (check_lt ^ check_rt)
	{
		if (check_lt)
		{
			MoveLT();
		}
		else if (check_rt)
		{
			MoveRT();
		}
	}
	if (check_up ^ check_dw)
	{
		if (check_up)
		{
			MoveUP();
		}
		else if (check_dw)
		{
			MoveDW();
		}
	}
}

void Player::Render(HDC canvas)
{
	Draw::Ellipse(canvas, x - 16, y - 16, x + 16, y + 16);
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
