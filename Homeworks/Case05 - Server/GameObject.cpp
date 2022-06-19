#include "pch.hpp"
#include "stdafx.hpp"
#include "GameObject.hpp"

GameObject::GameObject(PID id)
	: GameObject(id, 0.0f, 0.0f)
{}

GameObject::GameObject(PID id, float x, float y)
	: myID(id)
	, myPosition{ x, y }
{}

GameObject::GameObject(PID id, float pos[2])
	: GameObject(id, pos[0], pos[1])
{}

const float* GameObject::GetPosition() const
{
	return myPosition;
}

float* GameObject::GetPosition()
{
	return myPosition;
}
