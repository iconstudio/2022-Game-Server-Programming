#pragma once
#include "GameObject.hpp"
#include "SightSector.hpp"

class GameEntity : public GameObject
{
public:
	GameEntity(PID id, float x, float y, float z);
	GameEntity(PID id, const XMFLOAT3& pos);
	GameEntity(PID id, XMFLOAT3&& pos);
	virtual ~GameEntity();

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;

	void SetSightArea(const shared_ptr<SightSector>& sector);
	void SetSightArea(shared_ptr<SightSector>&& sector);

	PID myID;
	shared_ptr<SightSector> mySightSector;
};
