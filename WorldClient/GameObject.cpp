#include "pch.hpp"
#include "stdafx.hpp"
#include "GameObject.hpp"
#include "Sprite.hpp"

GameObject::GameObject()
	: mySprite(nullptr)
	, isVisible(true)
{}

GameObject::~GameObject()
{}

void GameObject::Render(HDC surface) const
{
	Render(surface, 0.0f, 0.0f);
}

void GameObject::Render(HDC surface, float ax, float ay) const
{}

void GameObject::Show()
{
	isVisible = true;
}

void GameObject::Hide()
{
	isVisible = false;
}
