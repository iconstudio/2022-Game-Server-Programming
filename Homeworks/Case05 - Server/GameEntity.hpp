#pragma once

class GameEntity
{
public:
	GameEntity(PID id, int x, int y);
	virtual ~GameEntity();

	PID myID;
	int x, y;
};
