#pragma once

class GameObject
{
public:
	GameObject();
	GameObject(float x, float y, float z);
	GameObject(float pos[3]);
	GameObject(const XMFLOAT3& pos);
	GameObject(XMFLOAT3&& pos);

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;

	const XMFLOAT3& GetPosition() const;
	XMFLOAT3& GetPosition();

	XMFLOAT3 myPosition;
};
