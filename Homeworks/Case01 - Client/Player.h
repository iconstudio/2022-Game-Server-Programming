#pragma once

class Player
{
public:
	void Update(float delta_time);
	void Render(HDC canvas);

	void MoveLT();
	void MoveRT();
	void MoveUP();
	void MoveDW();

	int x, y;
};
