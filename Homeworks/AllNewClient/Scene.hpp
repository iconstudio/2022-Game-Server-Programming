#pragma once

class Scene
{
public:
	Scene(Framework& framework, size_t instance_count);
	virtual ~Scene();

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface) = 0;

	virtual void OnNetwork(const Packet& packet);
	virtual void OnMouse(WPARAM button, LPARAM cursor);
	virtual void OnKeyboard(WPARAM key, LPARAM states);
	virtual void OnWindow(WPARAM aevent, LPARAM params);

	bool IsCompleted() const;
	bool IsPaused() const;

	bool TryPause();
	virtual void Pause();
	virtual void Resume();

private:
	virtual void Reset() = 0;
	virtual void Complete() = 0;

	Framework& myFramework;
	std::vector<shared_ptr<GameObject>> myInstances;

	bool isConsistent;
	bool isCompleted;
	bool isPaused;
};
