#include "pch.hpp"
#include "Commons.hpp"
#include "Packet.hpp"
#include "Draw.hpp"

SceneMain::SceneMain(Framework& framework)
	: Scene(framework, 0)
{}

void SceneMain::Awake()
{}

void SceneMain::Start()
{}

void SceneMain::Update(float time_elapsed)
{}

void SceneMain::Render(HDC surface)
{
	Draw::Clear(surface, CLIENT_W, CLIENT_H, C_WHITE);
}

void SceneMain::Reset()
{}

void SceneMain::Complete()
{}

SceneLoading::SceneLoading(Framework& framework)
	: Scene(framework, 0)
{}

void SceneLoading::Awake()
{}

void SceneLoading::Start()
{}

void SceneLoading::Update(float time_elapsed)
{}

void SceneLoading::Render(HDC surface)
{}

void SceneLoading::Reset()
{}

void SceneLoading::Complete()
{}

SceneGame::SceneGame(Framework& framework)
	: Scene(framework, 1000)
{}

void SceneGame::Awake()
{}

void SceneGame::Start()
{}

void SceneGame::Update(float time_elapsed)
{}

void SceneGame::Render(HDC surface)
{}

void SceneGame::Reset()
{}

void SceneGame::Complete()
{}
