#pragma once

class GameObject
{
public:
	GameObject(shared_ptr<Session> sesion);
	GameObject(shared_ptr<Session> sesion, shared_ptr<GameSprite> sprite);

	void SetSprite(const shared_ptr<GameSprite>& sprite);
	void SetSprite(shared_ptr<GameSprite>&& sprite);

	void Render(sf::RenderWindow& surface);
	void Render(sf::RenderWindow& surface, float view_x, float view_y);

	void show()
	{
		isVisible = true;
	}

	void hide()
	{
		isVisible = false;
	}

	void Jump(float x, float y)
	{
		m_x = x;
		m_y = y;
	}

	float m_x, m_y;

private:
	shared_ptr<Session> mySession;
	shared_ptr<GameSprite> mySprite;

	bool isVisible;
};

