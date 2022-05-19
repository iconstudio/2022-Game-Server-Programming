#include "stdafx.hpp"
#include "GameObject.hpp"

GameObject::GameObject()
	: GameObject(0.0f, 0.0f, 0.0f)
{}

GameObject::GameObject(float x, float y, float z)
	: GameObject(XMFLOAT3(x, y, z))
{}

GameObject::GameObject(float pos[3])
	: GameObject(XMFLOAT3(pos[0], pos[1], pos[3]))
{}

GameObject::GameObject(const XMFLOAT3& pos)
	: GameObject(XMFLOAT3(pos))
{}

GameObject::GameObject(XMFLOAT3&& pos)
	: myPosition(pos)
{}

const XMFLOAT3& GameObject::GetPosition() const
{
	return myPosition;
}

XMFLOAT3& GameObject::GetPosition()
{
	return myPosition;
}
