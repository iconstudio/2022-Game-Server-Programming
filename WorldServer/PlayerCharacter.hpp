#pragma once
#include "GameObject.hpp"

class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter(PID id);
	PlayerCharacter(PID id, float x, float y);
	PlayerCharacter(PID id, float pos[2]);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
