#include "pch.hpp"
#include "stdafx.hpp"
#include "GameObject.hpp"

GameObject::GameObject()
{}

GameObject::~GameObject()
{}

void GameObject::Render(HDC surface) const
{
	Render(surface, 0.0f, 0.0f);
}

void GameObject::Render(HDC surface, float ax, float ay) const
{}
