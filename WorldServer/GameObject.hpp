#pragma once

class GameObject
{
public:
	GameObject(PID id);
	GameObject(PID id, float x, float y);
	GameObject(PID id, float pos[2]);
	virtual ~GameObject();

	virtual void Awake();
	virtual void Start();
	virtual void Update(float time_elapsed);

	void SetBoundingBox(const RECT& box);
	void SetBoundingBox(RECT&& box);
	void SetBoundingBox(long left, long top, long right, long bottom);
	
	float GetBoundingLeft() const;
	float GetBoundingTop() const;
	float GetBoundingRight() const;
	float GetBoundingBottom() const;

	bool CheckCollision(const GameObject& other) const;
	bool CheckCollision(const GameObject* other) const;

	virtual bool TryMoveLT(float distance);
	virtual bool TryMoveRT(float distance);
	virtual bool TryMoveUP(float distance);
	virtual bool TryMoveDW(float distance);

	const float* GetPosition() const;
	float* GetPosition();

	PID myID;
	ENTITY_CATEGORY myCategory;
	ENTITY_TYPES myType;
	int myLevel;
	int myHP, myMaxHP;
	int myMP, myMaxMP;
	int myArmour;
	float myPosition[2];
	MOVE_TYPES myDirection;
	RECT myBoundingBox;
};
