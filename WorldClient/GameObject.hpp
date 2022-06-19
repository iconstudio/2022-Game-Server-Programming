#pragma once

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface) const;
	virtual void Render(HDC surface, float ax, float ay) const;

	virtual void Show();
	virtual void Hide();

	ENTITY_CATEGORY myCategory;
	ENTITY_TYPES myType;
	int myLevel;
	int myHP, myMaxHP;
	int myMP, myMaxMP;
	int myArmour;
	float myPosition[2];
	RECT myBoundingBox;

	shared_ptr<GameSprite> mySprite;
	bool isVisible;
};
