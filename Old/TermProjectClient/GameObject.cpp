#include "pch.hpp"
#include "GameObject.hpp"
#include "GameSprite.hpp"

GameObject::GameObject(shared_ptr<Session> sesion)
	: GameObject(sesion, nullptr)
{}

GameObject::GameObject(shared_ptr<Session> sesion, shared_ptr<GameSprite> sprite)
	: mySession(sesion), mySprite(sprite)
	, isVisible(true)
	, m_x(0.0f), m_y(0.0f)
{}

void GameObject::SetSprite(const shared_ptr<GameSprite>& sprite)
{
	mySprite = sprite;
}

void GameObject::SetSprite(shared_ptr<GameSprite>&& sprite)
{
	mySprite = std::forward<shared_ptr<GameSprite>>(sprite);
}

void GameObject::Render(sf::RenderWindow& surface)
{
	if (mySprite)
	{
		mySprite->Render(surface);
	}
}

void GameObject::Render(sf::RenderWindow& surface, float view_x, float view_y)
{
	if (!isVisible)
	{
		return;
	}

	float ax = (m_x - view_x) * 65.0f + 8;
	float ay = (m_y - view_y) * 65.0f + 8;

	if (mySprite)
	{
		mySprite->Render(surface, ax, ay);
	}
}
