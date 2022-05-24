#pragma once
#include "GameObject.hpp"

enum class ENTITY_TYPES : UCHAR
{
	NONE, NPC, QUEEST_NPC, MOB, BOSS, PLAYER
};

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
	XMFLOAT3 myPosition;
};
