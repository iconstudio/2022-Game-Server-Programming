#pragma once

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface) = 0;

	XMFLOAT3 myPosition;
};
