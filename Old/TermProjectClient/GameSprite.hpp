#pragma once

class GameSprite
{
public:
	GameSprite()
		: myImage()
	{}

	GameSprite(const Filepath& path, float ox = 0.0f, float oy = 0.0f)
	{
		if (!std::filesystem::exists(path))
		{
			throw "텍스쳐 파일을 찾을 수 없습니다!";
		}

		sf::Texture raw{};
		raw.loadFromFile(path.string());

		myImage.setOrigin(ox, oy);
		myImage.setTexture(raw);
	}

	GameSprite(const Filepath& path, float ox, float oy, int tx, int ty, int w, int h)
	{
		if (!std::filesystem::exists(path))
		{
			throw "텍스쳐 파일을 찾을 수 없습니다!";
		}

		sf::Texture raw{};
		raw.loadFromFile(path.string());

		myImage.setOrigin(ox, oy);
		myImage.setTexture(raw);
		myImage.setTextureRect(sf::IntRect(tx, ty, w, h));
	}

	GameSprite(const sf::Texture& atlas, float ox, float oy, int tx, int ty, int w, int h)
		: GameSprite()
	{
		myImage.setTexture(atlas);
		myImage.setTextureRect(sf::IntRect(tx, ty, w, h));
		myImage.setOrigin(ox, oy);
	}

	void Render(sf::RenderWindow& surface)
	{
		myImage.setRotation(0.0f);

		surface.draw(myImage);
	}

	void Render(sf::RenderWindow& surface, float ax, float ay)
	{
		myImage.setPosition(ax, ay);
		myImage.setRotation(0.0f);

		surface.draw(myImage);
	}

	void Render(sf::RenderWindow& surface, float ax, float ay, float rotation)
	{
		myImage.setPosition(ax, ay);
		myImage.setRotation(rotation);

		surface.draw(myImage);
	}

private:
	sf::Sprite myImage;
};
