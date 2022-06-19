#include "pch.hpp"
#include "stdafx.hpp"
#include "Framework.hpp"
#include "GameObject.hpp"

Framework gameFramework{ "Client", WINDOW_WIDTH, WINDOW_HEIGHT };

int main()
{
	std::wcout.imbue(std::locale("korean"));

	gameFramework.Awake();
	gameFramework.LoadSprite("sBoard", "chessmap.bmp", 0.0f, 0.0f, 0, 0, 64, 64);
	gameFramework.LoadSprite("sPieces", "chess2.png", 0.0f, 0.0f, 128, 0, 64, 64);

	gameFramework.Start("127.0.0.1", PORT_NUM);
	gameFramework.Update();

	gameFramework.Release();

	return 0;
}
