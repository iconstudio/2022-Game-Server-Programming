#pragma once

enum class ENTITY_TYPES : UCHAR
{
	NONE, NPC, QUEEST_NPC, MOB, BOSS, PLAYER
};

class GameObject
{
public:
	GameObject(PID id);
	GameObject(PID id, float x, float y);
	GameObject(PID id, float pos[2]);

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface);
	virtual void Render(HDC surface, float ax, float ay) = 0;

	const float* GetPosition() const;
	float* GetPosition();

	float myPosition[2];
	PID myID;
	ENTITY_TYPES myType;
};
