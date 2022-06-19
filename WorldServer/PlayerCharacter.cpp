#include "pch.hpp"
#include "PlayerCharacter.hpp"

PlayerCharacter::PlayerCharacter(PID id)
	: GameObject(id)
{}

PlayerCharacter::PlayerCharacter(PID id, float x, float y)
	: GameObject(id, x, y)
{}

PlayerCharacter::PlayerCharacter(PID id, float pos[2])
	: GameObject(id, pos[0], pos[1])
{}

void PlayerCharacter::Awake()
{}

void PlayerCharacter::Start()
{}

void PlayerCharacter::Update(float time_elapsed)
{}
