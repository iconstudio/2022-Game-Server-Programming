#pragma once

class GameObject
{
public:
	GameObject();
	GameObject(float x, float y, float z);
	GameObject(float pos[3]);
	GameObject(const XMFLOAT3& pos);
	GameObject(XMFLOAT3&& pos);

	const XMFLOAT3& GetPosition() const;
	XMFLOAT3& GetPosition();

	XMFLOAT3 myPosition;
};
