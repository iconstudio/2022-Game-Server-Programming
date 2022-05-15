#pragma once
#include "stdafx.hpp"
#include "GameTransform.hpp"

class GameObject
{
public:
	GameObject();
	GameObject(float positions[3]);
	GameObject(float x, float y, float z);
	virtual ~GameObject();

	void EnterSector(const shared_atomic<SightSector>& sector);
	void EnterSector(shared_atomic<SightSector>&& sector);
	void EnterSector(shared_ptr<SightSector>&& sector);
	void LeaveSector();

	XMFLOAT3& GetPositon();
	const XMFLOAT3& GetPosition() const;

	GameTransform myTransform;

	shared_atomic<SightSector> mySector;

private:
	//const std::vector<shared_ptr<GameObject>> mySiblings;
	//shared_ptr<GameObject> myChild;
};
