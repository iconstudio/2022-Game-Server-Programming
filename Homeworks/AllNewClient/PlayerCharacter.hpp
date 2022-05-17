#pragma once
#include "GameEntity.hpp"

class PlayerCharacter : public GameEntity
{
public:
	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
};
