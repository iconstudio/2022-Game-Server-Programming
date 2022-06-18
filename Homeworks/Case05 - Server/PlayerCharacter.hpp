#pragma once
#include "GameObject.hpp"

class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter(PID id, float x, float y, float z);
	PlayerCharacter(PID id, const XMFLOAT3& pos);
	PlayerCharacter(PID id, XMFLOAT3&& pos);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
