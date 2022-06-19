#include "pch.hpp"
#include "Session.hpp"
#include "GameObject.hpp"

Session::Session(PID id, bool is_player)
	: myID(id), myCharacter(nullptr)
	, isPlayer(is_player)
{}

void Session::Render(sf::RenderWindow& surface)
{
	if (myCharacter)
	{
		myCharacter->Render(surface);
	}
}

void Session::Render(sf::RenderWindow& surface, float view_x, float view_y)
{
	if (myCharacter)
	{
		myCharacter->Render(surface, view_x, view_y);

		const auto dx = (myCharacter->m_x - view_x) * 65.0f + 8.0f;
		const auto dy = (myCharacter->m_y - view_y) * 65.0f + 8.0f;

		if (m_mess_end_time < std::chrono::system_clock::now())
		{
			m_name.setPosition(dx - 10, dy - 20);

			surface.draw(m_name);
		}
		else
		{
			m_chat.setPosition(dx - 10, dy - 20);

			surface.draw(m_chat);
		}
	}
}
