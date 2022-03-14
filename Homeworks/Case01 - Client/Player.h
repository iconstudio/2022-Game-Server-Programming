#pragma once

class Player
{
public:
	void Update(float delta_time);
	void Render(HDC canvas);

	int x, y;
};
