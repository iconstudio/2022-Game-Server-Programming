#include "pch.hpp"
#include "stdafx.hpp"
#include "GameObject.hpp"

GameObject::GameObject(PID id)
	: GameObject(id, 0.0f, 0.0f)
{}

GameObject::GameObject(PID id, float x, float y)
	: myID(id)
	, myCategory(), myType()
	, myLevel(0)
	, myHP(1), myMaxHP(1)
	, myMP(0), myMaxMP(0)
	, myArmour(0)
	, myBoundingBox{ -16, -16, 16, 16 }
	, myPosition{ x, y }, myDirection(MOVE_TYPES::RIGHT)
{}

GameObject::GameObject(PID id, float pos[2])
	: GameObject(id, pos[0], pos[1])
{}

GameObject::~GameObject()
{}

void GameObject::Awake()
{}

void GameObject::Start()
{}

void GameObject::Update(float time_elapsed)
{}

void GameObject::SetBoundingBox(const RECT& box)
{
	myBoundingBox = box;
}

void GameObject::SetBoundingBox(RECT&& box)
{
	myBoundingBox = std::forward<RECT>(box);
}

void GameObject::SetBoundingBox(long left, long top, long right, long bottom)
{
	myBoundingBox.left = left;
	myBoundingBox.top = top;
	myBoundingBox.right = right;
	myBoundingBox.bottom = bottom;
}

float GameObject::GetBoundingLeft() const
{
	return myPosition[0] + static_cast<float>(myBoundingBox.left);
}

float GameObject::GetBoundingTop() const
{
	return myPosition[1] + static_cast<float>(myBoundingBox.top);
}

float GameObject::GetBoundingRight() const
{
	return myPosition[0] + static_cast<float>(myBoundingBox.right);
}

float GameObject::GetBoundingBottom() const
{
	return myPosition[1] + static_cast<float>(myBoundingBox.bottom);
}

bool GameObject::CheckCollision(const GameObject& other) const
{
	return !(other.GetBoundingRight() <= GetBoundingLeft()
		|| other.GetBoundingBottom() <= GetBoundingTop()
		|| GetBoundingRight() < other.GetBoundingLeft()
		|| GetBoundingBottom() < other.GetBoundingTop());
}

bool GameObject::CheckCollision(const GameObject* other) const
{
	return !(other->GetBoundingRight() <= GetBoundingLeft()
		|| other->GetBoundingBottom() <= GetBoundingTop()
		|| GetBoundingRight() < other->GetBoundingLeft()
		|| GetBoundingBottom() < other->GetBoundingTop());
}

bool GameObject::TryMoveLT(float distance)
{
	if (8.0f < myPosition[0] - distance)
	{
		myPosition[0] -= distance;
		return true;
	}
	else if (myPosition[0] != 8.0f)
	{
		myPosition[0] = 8.0f;
		return true;
	}
	return false;
}

bool GameObject::TryMoveRT(float distance)
{
	if (myPosition[0] + distance < WORLD_W - 8.0f)
	{
		myPosition[0] += distance;
		return true;
	}
	else if (myPosition[0] != WORLD_W - 8.0f)
	{
		myPosition[0] = WORLD_W - 8.0f;
		return true;
	}
	return false;
}

bool GameObject::TryMoveUP(float distance)
{
	if (8.0f < myPosition[1] - distance)
	{
		myPosition[1] -= distance;
		return true;
	}
	else if (myPosition[1] != 8.0f)
	{
		myPosition[1] = 8.0f;
		return true;
	}
	return false;
}

bool GameObject::TryMoveDW(float distance)
{
	if (myPosition[1] + distance < WORLD_H - 8.0f)
	{
		myPosition[1] += distance;
		return true;
	}
	else if (myPosition[1] != WORLD_H - 8.0f)
	{
		myPosition[1] = WORLD_H - 8.0f;
		return true;
	}
	return false;
}

const float* GameObject::GetPosition() const
{
	return myPosition;
}

float* GameObject::GetPosition()
{
	return myPosition;
}
