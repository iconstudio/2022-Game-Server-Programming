#pragma once
#include "GameObject.hpp"
#include "PlayerCharacter.hpp"
#include "GameCamera.hpp"

class Scene
{
public:
	Scene(Framework& framework, const char* name, size_t instance_count = 0);
	virtual ~Scene();

	template<typename Type, typename Op = std::remove_cvref_t<Type>, bool = std::is_base_of_v<GameObject, Op>>
	Op* CreateInstance();
	template<>
	GameEntity* CreateInstance<GameEntity, GameEntity, true>();
	template<>
	PlayerCharacter* CreateInstance<PlayerCharacter, PlayerCharacter, true>();


	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface) = 0;

	bool IsCompleted() const;
	bool IsPaused() const;

	bool TryPause();
	virtual void Pause();
	virtual void Resume();

	[[nodiscard]] virtual bool OnNetwork(const Packet& packet);
	virtual void OnMouse(UINT type, WPARAM button, LPARAM cursor);
	virtual void OnKeyboard(UINT type, WPARAM key, LPARAM states);
	virtual void OnKeyDown(WPARAM key, LPARAM states);
	virtual void OnKeyUp(WPARAM key, LPARAM states);
	virtual void OnWindow(WPARAM aevent, LPARAM params);

	const string myName;

protected:
	virtual void Reset() = 0;
	virtual void Complete() = 0;

	void AddInstance(GameObject* instance);

	Framework& myFramework;
	GameCamera myCamera;

	std::vector<shared_ptr<GameObject>> myInstances;

	bool isConsistent;
	bool isCompleted;
	bool isPaused;
};
