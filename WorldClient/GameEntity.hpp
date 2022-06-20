#pragma once
#include "GameObject.hpp"

class GameEntity : public GameObject
{
public:
	GameEntity();
	virtual ~GameEntity();

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float time_elapsed) override;
	virtual void Render(HDC surface) const override;
	virtual void Render(HDC surface, float ax, float ay) const override;

	PID myID;
};
