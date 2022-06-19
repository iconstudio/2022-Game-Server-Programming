#pragma once

class Session
{
public:
	Session(PID id, bool is_player);

	void Render(sf::RenderWindow& surface);
	void Render(sf::RenderWindow& surface, float view_x, float view_y);

	void set_name(const char str[])
	{
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}

	void SetNameFont(const sf::Font& font)
	{
		m_name.setFont(font);
	}

	void set_chat(const char str[])
	{
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);

		m_mess_end_time = std::chrono::system_clock::now() + std::chrono::seconds(3);
	}

	void SetChattingFont(const sf::Font& font)
	{
		m_chat.setFont(font);
	}

	PID myID;
	shared_ptr<GameObject> myCharacter;
	const bool isPlayer;

	sf::Text m_name;
	sf::Text m_chat;
	std::chrono::system_clock::time_point m_mess_end_time;
};
