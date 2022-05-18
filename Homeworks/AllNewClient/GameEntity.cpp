#include "pch.hpp"
#include "stdafx.hpp"
#include "GameEntity.hpp"

GameEntity::GameEntity()
	: GameObject()
{}

GameEntity::~GameEntity()
{}

void GameEntity::Awake()
{}

void GameEntity::Start()
{}

void GameEntity::Update(float time_elapsed)
{}

void GameEntity::Render(HDC surface) const
{
	GameEntity::Render(surface, 0.0f, 0.0f);
}

void GameEntity::Render(HDC surface, float ax, float ay) const
{}
