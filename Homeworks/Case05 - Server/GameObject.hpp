#pragma once

class GameObject
{
public:
	GameObject();
	GameObject(float x, float y, float z);
	GameObject(float pos[3]);
	GameObject(const XMFLOAT3& pos);
	GameObject(XMFLOAT3&& pos);

	XMFLOAT3 myPosition;
};
