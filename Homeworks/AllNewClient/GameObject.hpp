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
};
