#include "stdafx.hpp"
#include "GameObject.hpp"
#include "SightSector.hpp"

GameObject::GameObject()
	: GameObject(0.0f, 0.0f, 0.0f)
{}

GameObject::GameObject(float positions[3])
	: GameObject(positions[0], positions[1], positions[2])
{}

GameObject::GameObject(float x, float y, float z)
	: myTransform(x, y, z)
{}

GameObject::~GameObject()
{}

void GameObject::EnterSector(const shared_atomic<SightSector>& sector)
{
	EnterSector(sector.load());
}

void GameObject::EnterSector(shared_atomic<SightSector>&& sector)
{
	EnterSector(std::forward<shared_atomic<SightSector>>(sector).load());
}

void GameObject::EnterSector(shared_ptr<SightSector>&& sector)
{
	mySector = std::forward<shared_ptr<SightSector>>(sector);
}

void GameObject::LeaveSector()
{
	auto&& my_sector = mySector.load();
	if (my_sector)
	{
		mySector.store(nullptr);
	}
}

XMFLOAT3& GameObject::GetPositon()
{
	return myTransform.myPosition;
}

const XMFLOAT3& GameObject::GetPosition() const
{
	return myTransform.myPosition;
}
